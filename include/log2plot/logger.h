#ifndef LOG2PLOTLOGGER_H_
#define LOG2PLOTLOGGER_H_

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <log2plot/container.h>
#include <cmath>

namespace log2plot
{

// legend for a number of points (x_i,y_i)
std::string legend2DPoint(const unsigned int &n=4);

const double nan = std::nan("");

template<class T>
void setNaN(T& v, uint start = 0, uint end = 0)
{

  if(start == 0 && end == 0)
    end = v.size();
  for(uint i = start; i < end; ++i)
    v.operator[](i) = nan;
}

class Logger
{
protected:

    std::string file_path;

    // related to high-level functions
    unsigned int buff, subsamp, buff_count = 0, iter_count = 0;
    double * time = nullptr;
    std::string time_unit = "s";
    std::vector<double> time_buff;
    std::vector<double> steps, steps_timed;

    // logger variables and pointer to the last entered
    std::vector<std::unique_ptr<GenericContainer> > logged_vars;
    GenericContainer* last;

    bool first_update = true;

    // write initial information in latest saved variable
    void writeInitialInfo(const LogType &log_type, const std::string &name, const std::string &legend, const std::string &xlabel, const std::string &ylabel, const bool &keep_file);

    // build explicit legend from implicit
    static std::string buildLegend(const std::string legend, const unsigned int len);

public:
    // constructor with default values
    Logger(std::string _file_path = "", unsigned int _buffer = 10, unsigned int _subsampling = 1)
      :  file_path(_file_path), buff(_buffer), subsamp(_subsampling)
    {
        logged_vars.clear();
        steps.clear();
        steps_timed.clear();
    }

    ~Logger()
    {
      for(auto &var: logged_vars)
        var->close(steps, steps_timed);
    }

    // **** Parameter methods ****

    // Set pointer to time variable
    inline void setTime(double &t, const std::string &unit = "s") {time = &t;time_unit = unit;}
    // number of calls before flushing to file
    inline void setBuffer(unsigned int b) {buff = b;}
    // if any subsampling
    inline void setSubSampling(unsigned int s) {subsamp = s;}
    // path to files to be saved, can include any prefix for files
    inline void setSavePath(std::string _file_path) {file_path = _file_path;}

    // **** Functions to add new variables to be saved ****

    // Save iteration-based vector
    template<class T>
    inline void save(T &v, const std::string &name, const std::string &legend, const std::string &ylabel, bool keep_file = true)
    {
        // add this to logged variables
        logged_vars.push_back(std::unique_ptr<Container<T> >(new Container<T>(v)));
        // and write initial info
        writeInitialInfo(log2plot::ITERATION, name, buildLegend(legend, v.size()), "iterations", ylabel, keep_file);
    }

    // Save time-based vector
    template<class T>
    inline void saveTimed(T &v, const std::string &name, const std::string &legend, const std::string &ylabel, bool keep_file = true)
    {
        // add this to logged variables
        logged_vars.push_back(std::unique_ptr<Container<T> >(new Container<T>(v)));
        // and write initial info
        writeInitialInfo(log2plot::TIME, name, buildLegend(legend, v.size()), "time [" + time_unit + "]", ylabel, keep_file);
    }

    // Save XY vector
    template<class T>
    inline void saveXY(T &v, const std::string &name, const std::string &legend, const std::string &xlabel, const std::string &ylabel, bool keep_file = true)
    {
        // add this to logged variables
        logged_vars.push_back(std::unique_ptr<Container<T> >(new Container<T>(v)));
        // and write initial info
        writeInitialInfo(log2plot::XY, name, buildLegend(legend, v.size()/2), xlabel, ylabel, keep_file);
    }

    // Save 3D pose or position
    template<class T>
    inline void save3Dpose(T &v, const std::string &name, const std::string &legend, bool invert = false, bool keep_file = true)
    {
        // add this to logged variables
        logged_vars.push_back(std::unique_ptr<Container<T> >(new Container<T>(v)));
        // and write initial info
        writeInitialInfo(log2plot::POSE, name, "["+legend+"]", "", "", keep_file);
        if(invert)
            last->writeInfo("invertPose", "True");
    }

    // **** End functions for new variables ****

    // **** Functions to specify metadata for the last registered variable ****

    // Units
    void setUnits(const std::string units) {last->writeInfo("units", units);}
    // Line types
    void setLineType(const std::string lineType) {last->writeInfo("lineType", lineType);}

    // Dashed steps
    void setSteps(std::vector<double> _steps)
    {
      last->setSteps(_steps);
    }

    // 3D plot: show camera
    void showMovingCamera(const std::vector<double> &desired_pose = std::vector<double>(),const double &x = 1.5, const double &y = 1, const double &z = 4);
    // 3D plot: show box
    void showMovingBox(const double &x = 10, const double &y = 5, const double &z = 3, const std::vector<double> &desired_pose = std::vector<double>());
    // 3D plot: custom object with a (nx3) matrix
    void showMovingObject(const std::vector<std::vector<double> > &M, const std::string &graph, const std::vector<double> &desired_pose = std::vector<double>());

    // 3D plot: fixed object (related to object frame)
    void showFixedObject(const std::vector<std::vector<double> > &M, const std::string &graph, const std::string &color = "");
    // 3D plot: fixed 3D-box
    void showFixedBox(const double &xm, const double &ym, const double &zm, const double &xM, const double &yM, const double &zM, const std::string &color = "");
    // 3D plot: fixed 2D-rectangle on Z=0
    void showFixedRectangle(const double &xm, const double &ym, const double &xM, const double &yM, const std::string &color = "");
    // **** End metadata functions ****

    // Add a time-step
    void writeStep()
    {
      steps.push_back(iter_count / subsamp);
      if(time)
        steps_timed.push_back(*time);
    }

    // Updates all saved variables
    void update(const bool &flush = false);

    // Low-level static functions
    // Build 3D box nodes depending on dimensions
    static std::vector<std::vector<double> > buildBoxNodes(const double &xm, const double &ym, const double &zm, const double &xM, const double &yM, const double &zM);
    // Builds YAML-proof single-lined matrix
    static std::string toYAMLVector(const std::vector<std::vector<double> > &M);

    // Close all files and call the Python interpreter on the given script path
    void plot(std::string script_path, bool verbose = false);
    void plot(bool verbose = false)
    {
      plot("", verbose);
    }

};
}

#endif /* LOG2PLOTLOGGER_H_ */
