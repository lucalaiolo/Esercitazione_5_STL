#include "Utils.hpp"
#include <iostream>
#include <Eigen/Eigen>
#include <fstream>
#include <sstream>
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
        // Test
        //cout << mesh.NumberCell2D;
        for(unsigned int c = 0; c < mesh.NumberCell2D; c++) {
            vector<unsigned int> edges = mesh.Cell2DEdges[c]; // estraggo i lati dal poligono c-esimo
            const unsigned int numEdges = edges.size();
            const unsigned int numVertices = mesh.Cell2DVertices[c].size();
            if(numEdges != numVertices) {
                cout << "There is a Cell 2D that is not a proper polygon" << endl;
                return false;
            }
            for(unsigned int e=0;e<numEdges;e++) { // estraggo gli id dei singoli lati
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
        for(int i=0;i<mesh.NumberCell2D;i++){
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
            unsigned int dummyVar; // dummy variable that will contain the marker of the 2D cell (they are all equal to 0)
            unsigned int numVertices;
            unsigned int numEdges;
            convert >> id >> dummyVar >> numVertices; // line 182 makes this work
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



