#include "geometry.h"

using namespace std;

enum ObstacleType {wall, single_mirror, double_mirror};
class ObstacleForSim;
class LightFrustrumForSim;

vector<LightFrustrum> run_light_simulation(vector<ObstacleForSim> obstacles, LightFrustrumForSim startLight);
void test_simulation();
