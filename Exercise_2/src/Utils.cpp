#include "Utils.hpp"
#include <iostream>
#include <Eigen/Eigen>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
namespace PolygonalLibrary {

bool ImportMesh(const string &filename, PolygonalMesh &mesh) {
    if(!ImportCell0Ds(filename + "/Cell0Ds.csv", mesh)) {
        return false;
    }
    else {
        cout << "Cell0D marker: " << endl;
        for(auto& [key,val] : mesh.Cell0DMarkers) {
            cout << "marker:\t" << key << "\t ids:";
            for(auto& id : val)
                cout << "\t" << id;
            cout << endl;
        }
    }

    if(!ImportCell1Ds(filename + "/Cell1Ds.csv", mesh)) {
        return false;
    } else {
        cout << "Cell 1D marker: " << endl;
        for(auto& [marker,ids] : mesh.Cell1DMarkers) {
            cout << "Marker:\t" << marker << "\t ids:";
            for(auto& id:ids) {
                cout << "\t" << id;
            }
            cout << endl;
        }
    }

    if(!ImportCell2Ds(filename + "/Cell2Ds.csv", mesh)) {
        return false;
    } else {
        // Test 1
        //cout << mesh.NumberCell2D;
        for(unsigned int c = 0; c < mesh.NumberCell2D; c++) {
            vector<unsigned int> edges = mesh.Cell2DEdges[c]; // extract the edges of the c-th polygon
            const unsigned int numEdges = edges.size();
            const unsigned int numVertices = mesh.Cell2DVertices[c].size();
            if(numEdges != numVertices) {
                cout << "There is a Cell 2D that is not a proper polygon" << endl;
                return false;
            }
            for(unsigned int e=0;e<numEdges;e++) { // extract the edges ids
                const unsigned int origin = mesh.Cell1DVertices[edges[e]][0];
                const unsigned int end = mesh.Cell1DVertices[edges[e]][1];

                auto findOrigin = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(),origin);
                if(findOrigin == mesh.Cell2DVertices[c].end()) {
                    cout << "Wrong mesh" << endl;
                    return false;
                }
                auto findEnd = find(mesh.Cell2DVertices[c].begin(), mesh.Cell2DVertices[c].end(),end);
                if(findEnd == mesh.Cell2DVertices[c].end()) {
                    cout << "Wrong mesh" << endl;
                    return false;
                }
            }
        }

        // Let's print all the information related to the 2D cells
        cout << "2D Cells: " << endl;
        for(unsigned int i=0;i<mesh.NumberCell2D;i++){
            cout << i<< ". Id:\t" << mesh.Cell2DId[i] << endl;
            cout << "NumVertices:\t" << mesh.Cell2DVertices[i].size() << endl;
            cout << "Vertices Ids:";
            for(auto& id : mesh.Cell2DVertices[i]) {
                cout << "\t" << id;
            }
            cout << endl;
            cout << "NumEdges:\t" << mesh.Cell2DEdges[i].size() << endl;
            cout << "Edges Ids:";
            for(auto& id : mesh.Cell2DEdges[i]){
                cout << "\t" << id;
            }
            cout << endl << endl;
        }
        // 2D cell information stored correctly

        const double geometric1Dtol = 1.0e-12;
        // Test 2: length of edges is non zero
        for(unsigned int e=0;e<mesh.NumberCell1D;e++) {// iterate on edges' ids
            const unsigned int origin = mesh.Cell1DVertices[e][0];
            const unsigned int end = mesh.Cell1DVertices[e][1]; // (origin,end)==(fromId,toId) in mesh.Cell1DVertices[e]
            const Vector2d origin_coord = mesh.Cell0DCoordinates[origin];
            const Vector2d end_coord = mesh.Cell0DCoordinates[end];
            const double length = sqrt((origin_coord(0)-end_coord(0))*(origin_coord(0)-end_coord(0))+(origin_coord(1)-end_coord(1))*(origin_coord(1)-end_coord(1)));
            if(length < geometric1Dtol) {
                cout << "There is an edge shorter than the 1D geometric tolerance." << endl;
                return false;
            }
        }

        // Test 3: area of triangles is non zero
        const double geometric2Dtol = (sqrt(3.)/4.)*geometric1Dtol*geometric1Dtol;
        // the minimum area is chosen to be the area of the equilateral triangle with sides' length = to geometric1Dtol
        // we use properties of the cross product
        for(unsigned int c=0; c<mesh.NumberCell2D;c++) { // iterate on every polygon
            if(mesh.Cell2DEdges[c].size()==3) { // we extract the triangles
                vector<unsigned int> id_vertices = mesh.Cell2DVertices[c];
                Vector2d coord1 = mesh.Cell0DCoordinates[id_vertices[0]];
                Vector2d coord2 = mesh.Cell0DCoordinates[id_vertices[1]];
                Vector2d coord3 = mesh.Cell0DCoordinates[id_vertices[2]];
                if(0.5*abs((coord2(0)-coord1(0))*(coord3(1)-coord1(1))-(coord3(0)-coord1(0))*(coord2(1)-coord1(0))) < geometric2Dtol) {
                    cout << "There is a triangle whose area is smaller than the 2D geometric tolerance." << endl;
                    return false;
                }
            }

        }

    }
    return true;
}

bool ImportCell0Ds(const string &filename, PolygonalMesh &mesh) {

    ifstream file;
    file.open(filename);
    if(file.fail()) {
        return false;
    }

    list<string> listLines;
    string line;
    while(getline(file,line)) {
        std::replace(line.begin(),line.end(),';',' ');
        listLines.push_back(line);
    }
    listLines.pop_front();
    file.close();

    mesh.NumberCell0D = listLines.size();
    if(mesh.NumberCell0D == 0) {
        cout << "There is no cell 0D" << endl;
        return false;
    }

    mesh.Cell0DId.reserve(mesh.NumberCell0D);
    mesh.Cell0DCoordinates.reserve(mesh.NumberCell0D);

    for(const string& line:listLines) {
        istringstream convert(line);
        unsigned int id;
        unsigned int marker;
        Vector2d coordinates;
        convert >> id >> marker >> coordinates(0) >> coordinates(1); // line 96 makes this work

        mesh.Cell0DId.push_back(id);
        mesh.Cell0DCoordinates.push_back(coordinates);

        if(marker!=0) {
            auto ret = mesh.Cell0DMarkers.insert({marker, {id}});
            if(!ret.second)
                (*(ret.first)).second.push_back(id);
        }
    }
    return true;
}

bool ImportCell1Ds(const string &filename, PolygonalMesh &mesh) {
    ifstream file;
    file.open(filename);
    if(file.fail()) {
        return false;
    } else {
        list<string> listLines;
        string line;
        while(getline(file,line)) {
            std::replace(line.begin(),line.end(),';',' ');
            listLines.push_back(line);
        }
        listLines.pop_front();
        file.close();
        mesh.NumberCell1D = listLines.size();
        if(mesh.NumberCell1D == 0) {
            cout << "There is no cell 1D." << endl;
            return false;
        } else {
            mesh.Cell1DId.reserve(mesh.NumberCell1D);
            mesh.Cell1DVertices.reserve(mesh.NumberCell1D);

            for(string& line:listLines) {
                istringstream convert(line);
                unsigned int id;
                unsigned int marker;
                Vector2i vertices;
                convert >> id >> marker >> vertices(0) >> vertices(1); // line 139 makes this work
                mesh.Cell1DId.push_back(id);
                mesh.Cell1DVertices.push_back(vertices);

                if(marker!=0) {
                    auto ret = mesh.Cell1DMarkers.insert({marker, {id}});
                    if(!ret.second) {
                        (*(ret.first)).second.push_back(id);
                    }
                }
            }
        }
        return true;
    }
}

bool ImportCell2Ds(const string &filename, PolygonalMesh &mesh) {
    ifstream file;
    file.open(filename);
    if(file.fail()) {
        return false;
    } else {
        string line;
        list<string> listLines;
        while(getline(file,line)) {
            std::replace(line.begin(),line.end(),';',' ');
            listLines.push_back(line);
        }
        file.close();
        listLines.pop_front();
        mesh.NumberCell2D = listLines.size();
        if(mesh.NumberCell2D == 0) {
            cout << "There is no cell 2D" << endl;
            return false;
        }
        mesh.Cell2DId.reserve(mesh.NumberCell2D);
        mesh.Cell2DVertices.reserve(mesh.NumberCell2D);
        mesh.Cell2DEdges.reserve(mesh.NumberCell2D);
        for(const string& line : listLines) {
            istringstream convert(line);
            unsigned int id;
            unsigned int unused; // unused variable that will contain the marker of the 2D cell (they are all equal to 0)
            unsigned int numVertices;
            unsigned int numEdges;
            convert >> id >> unused >> numVertices; // line 182 makes this work
            mesh.Cell2DId.push_back(id);
            vector<unsigned int> vertices(numVertices);
            for(unsigned int i=0; i<numVertices; i++) {
                convert >> vertices[i];
            }
            mesh.Cell2DVertices.push_back(vertices);

            convert >> numEdges;
            vector<unsigned int> edges(numEdges);
            for(unsigned int i=0;i<numEdges;i++) {
                convert >> edges[i];
            }
            mesh.Cell2DEdges.push_back(edges);
        }
    }
    return true;
}
}



