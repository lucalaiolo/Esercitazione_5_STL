#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include "Eigen/Eigen"
#include <fstream>
using namespace std;
using namespace Eigen;
using namespace PolygonalLibrary;

int main()
{
    PolygonalMesh mesh;
    const string filepath = "./PolygonalMesh";
    if(!ImportMesh(filepath,mesh)) {
        return false;
    }
  return 0;
}
