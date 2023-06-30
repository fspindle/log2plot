#include <log2plot/logger.h>
#include <random>
#include <algorithm>

using Point = std::array<double, 3>;

inline double randCoord()
{
  static std::default_random_engine engine;
  static auto gen{std::uniform_real_distribution()};
  return -5 + 10 * gen(engine);
}

inline Point randomPoint()
{
  return Point{randCoord(), randCoord(), randCoord()};
}

inline std::vector<Point> generateSphere(double rad, double x0, double y0, double z0)
{
  const auto n{100};
  std::vector<Point> sphere;
  sphere.reserve(n);

  // use golden spiral
  static const double angle_incr(4*M_PI/(1 + sqrt(5)));
  const auto n_inv(1./n);

  for(uint i = 0; i < n; ++i)
  {
    const double z(i*2*n_inv + n_inv - 1);
    const double rho(sqrt(1-z*z));
    const double angle(i*angle_incr);
    sphere.push_back({x0 + rad*rho*cos(angle),
                      y0 + rad*rho*sin(angle),
                      z0 + rad*z});
  }
  return sphere;
}

inline std::vector<Point> generateTorus(double r, double R,
                                        double x0, double y0, double z0)
{
  const auto steps{100.};
  std::vector<Point> torus;
  torus.reserve(steps * steps);
  const auto incr{2*M_PI/steps};

  for(auto ui = 0; ui < steps; ++ui)
  {
    const auto cu{cos(ui*incr)};
    const auto su{sin(ui*incr)};

    for(auto vi = 0; vi < steps; ++vi)
    {
      const auto cv{cos(vi*incr)};
      const auto sv{sin(vi*incr)};
      const auto dx{(R+r*cv)*cu};
      const auto dy{(R+r*cv)*su};
      const auto dz{r*sv};

      torus.push_back({x0+dx, y0+dy, z0+dz});
    }
  }
  return torus;
}



int main()
{
  log2plot::Logger logger(LOG2PLOT_EXAMPLE_PATH);

  std::vector<double> dummy(3);
  logger.save3Dpose(dummy, "static_3d");
  logger.setPlotArgs("--legendCol 2");

  std::vector<Point> cloud(40);

  // some points
  std::generate(cloud.begin(), cloud.end(), randomPoint);
  logger.showFixedObject(cloud, "[]", "C0d", "a discrete object");

  // fully connected
  cloud.resize(10);
  std::generate(cloud.begin(), cloud.end(), randomPoint);
  logger.showFixedObject(cloud, log2plot::legendFullyConnected(cloud.size()), "C1", "random lines");

  // builtin
  logger.showFixedBox(1, 1, 1, 3, 4, 5, "C2", "a box");

  // surface
  cloud = generateTorus(0.2, 1., 4, -4, -4);
  logger.showFixedObject(cloud, "", "C4", "a torus with reconstructed surface");
  logger.displayObjectAs(log2plot::Surface(.3));

  cloud = generateSphere(1, -4, -4, -4);
  logger.showFixedObject(cloud, "", "C5", "a sphere with alpha shape");
  logger.displayObjectAs(log2plot::AlphaShape(1., .3));

  cloud = generateTorus(0.3, 1, -4, 4, -4);
  logger.showFixedObject(cloud, "", "C6", "a torus with convex hull");
  logger.displayObjectAs(log2plot::ConvexHull(.3));

  logger.plot();
}
