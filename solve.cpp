#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <tuple>
#include <iterator>
#include <omp.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <boost/graph/graphviz.hpp>
#include <boost/range.hpp>
#include <boost/graph/copy.hpp>
#include <boost/filesystem.hpp>
#include <bits/stdc++.h>

using namespace boost;
using namespace std;
int NNODES;
int cost;

#define THREADS 8



struct VertexData {
    int idVertex;
    int colorVertex;
    bool HeadChain;
    bool TailChain;
};

struct EdgeData {
    int colorEdge;
};


typedef adjacency_list<vecS, vecS, bidirectionalS,VertexData,EdgeData> Graph;

class Record
{
    public:
        Graph g;
        int appeared;
   
        Record(Graph g, int appeared)
        {
            this->g = g;
            this->appeared = appeared;
        }
};

namespace boost {
    namespace serialization {
        template <class Archive> void serialize(Archive &ar, VertexData &vd, unsigned /*version*/) {
            ar & vd.idVertex;
            ar & vd.colorVertex;
            ar & vd.HeadChain;
            ar & vd.TailChain;
        }
        template <class Archive> void serialize(Archive &ar, EdgeData &ed, unsigned /*version*/) {
            ar & ed.colorEdge;
        }
    } // namespace serialization
} // namespace boost

template <class Name>
class VertexWriter {
public:
    VertexWriter(Name _name) : name(_name) {}
    template <class Vertex>
    void operator()(std::ostream& out, const Vertex& v) const {
        auto item = name[v];
    std::string color;
            if (item.colorVertex == 0) {
                color = "green";
            } else {
                color = "red";
            }
            out << "[label=\"" << item.idVertex << "\" fillcolor=\"" << color << "\" style=\"filled\"]";
    }
private:
    Name name;
};


template <class Name>
class EdgeWriter {
public:
    EdgeWriter(Name _name) : name(_name) {}
    template <class Edge>
    void operator()(std::ostream& out, const Edge& e) const {
            auto item = name[e];
            std::string color;
            if (item.colorEdge == 0) {
                color = "green";
            } else {
                color = "red";
            }
            out << "[label=\"" << item.colorEdge << "\" color=\"" << color << "\"]";
    }
private:
    Name name;
};

bool graphEqual(Graph g1,Graph g2){
    if (num_vertices(g1) != num_vertices(g2)) return false;
    if (num_edges(g1) != num_edges(g2)) return false;
    for (auto ed : boost::make_iterator_range(edges(g1))) {
        if (target(ed,g1) != target(ed,g2) || source(ed,g1) != source(ed,g2)) {
            return false;
        }
    }
    return true;

}


// function TargetValueSearchBFS based on bfs to search if it exists a path of a fixed cost (maybe use a a* heuristic to make the function faster)
bool TVSB(Graph &g,int source,int targt /* target */, int costPath) {
  if (source==targt && costPath == 0)
    return true;
  else {
    if (costPath>0){
      for (auto ed : make_iterator_range(boost::out_edges(source, g))){
        if (TVSB(g,target(ed, g),targt,costPath-g[ed].colorEdge) == true)
          return true;
      }
      return false;
    } else {
      return false;
    }
  }
}


void addToList(Graph &g,int appeared,vector<Record> &gList){
    // we should use this function with completeGraph but i will still use it to store uncompleted graphs (don't know if the redundancy test is usefull)
    for (int i = 0; i < gList.size(); i++){
        if (graphEqual(g,gList[i].g)){
            gList[i].appeared+= appeared; // verify that we have a good amount of graphs
            return;
        }
    }
    gList.push_back(Record(g,appeared));
}

void completeGraph(Graph &g,vector<pair<Graph,int>> &gList){
    // i don't find any interest of this function, il just generates all the possible paths that are useless for properties.
    // maybe we can just ignore that and store our graph not completed to save memory and time
}

void nextPath(Graph &g,vector<Record> &gList);

void MakePath(Graph &g,int currentNode, int targt, int costPath, vector<Record> &gList){
    if (costPath<cost-1) {
        // iterate over all out edges
        bool isfixed = false;
        int nextNode;
        for (auto ed : make_iterator_range(boost::out_edges(currentNode, g))){
            // if it's a fixed red edge 
            if (g[ed].colorEdge == 1){
                MakePath(g,target(ed,g),targt,costPath+1,gList);
                isfixed = true;
            // if it's a fixed green edge
            } else if (g[ed].colorEdge == 0){
                MakePath(g,target(ed,g),targt,costPath,gList);
            }       
        }

        // if a red edge is fixed we need to add an edge and split our tree with n branches with n the number of nodes where we can go with a red link
        if (!isfixed){
            if (g[currentNode].TailChain == 1){
                // this is a tail so we need to connect the red vertice to antoher red vertice
                for (int vt=1;vt<num_vertices(g);vt+=2){
                    bool hasPath = false;
                    for (auto ed : make_iterator_range(in_edges(vt,g))){
                        if (g[ed].colorEdge ==1){
                            hasPath = true;
                            break;
                        }
                    }
                    if (hasPath == false){
                        auto ed = add_edge(currentNode,vt,g).first;
                        g[ed].colorEdge = 1;
                        MakePath(g,vt,targt,costPath+1,gList);
                        remove_edge(ed,g);
                    }
                }
            } else {
                // on peut faire en sorte d'optimiser un peu plus en séparant en 2 blocs, d'aller vers tout les rouge cycle ou chain, ou alors aller vers une tête verte de chaine 
                for (auto vt : make_iterator_range(vertices(g))){

                    bool hasPath = false;
                    for (auto ed : make_iterator_range(in_edges(vt,g))){
                        // on va venir vérifier qu'il n'y a pas déjà une liaison rouge entrante vers là où on veut.
                        if (g[ed].colorEdge ==1){
                            hasPath = true;
                            break;
                        }
                    }

                    if (hasPath == false){
                        // si jamais le lien vers lequel on dirige est une tête de chaine on vérifie qu'on à la bonne couleur
                        if (g[vt].HeadChain == 1){
                            if (currentNode%2!=0){
                                auto ed = add_edge(currentNode,vt,g).first;
                                g[ed].colorEdge = 1;
                                MakePath(g,vt,targt,costPath+1,gList);
                                remove_edge(ed,g);
                            } 
                        } else {
                            auto ed = add_edge(currentNode,vt,g).first;
                            g[ed].colorEdge = 1;
                            MakePath(g,vt,targt,costPath+1,gList);
                            remove_edge(ed,g);
                        }
                    }
                }
            }
        }
    } else {
        if (currentNode == targt){
            nextPath(g,gList);
        }
    }
}

void nextPath(Graph &g,vector<Record> &gList){
    // en faisant l'optimisation à cost-1 et en partant uniquement des rouges pour arriver aux verts on a normalement des rés
    for (int source = 1; source < num_vertices(g); source+=2){
        for (int targt = 0; targt < num_vertices(g); targt+=2){
            if (TVSB(g,source,targt,cost-1) == false){
                //cout << "new path from " << source << " to "<< target << endl;
                MakePath(g,source,targt,0,gList);
                return;
            }
        }
    }
    //cout << "solution found !" << endl;
    

    /*
    VertexWriter<Graph> vw(g);
    EdgeWriter<Graph> ew(g);
    write_graphviz(std::cout, g, vw, ew);
    */
    
    
    addToList(g,1,gList);
}


void displayMD(Graph &g,vector<Record> &GraphList,int batchSize,int n,string id) {
    VertexWriter<Graph> vertexWriter(g);
    EdgeWriter<Graph> edgeWriter(g);
    ofstream markdown;

    boost::filesystem::path dir("markdowns/"+to_string(NNODES)+"-nodes/");

    if(!(exists(dir))){
        std::cout<<"Doesn't Exists"<<std::endl;

        if (create_directory(dir))
            std::cout << "....Successfully Created !" << std::endl;
    }

    boost::filesystem::path dir2("markdowns/"+to_string(NNODES)+"-nodes/solutions-"+id+"/");

    if(!(exists(dir2))){
        std::cout<<"Doesn't Exists"<<std::endl;

        if (create_directory(dir2))
            std::cout << "....Successfully Created !" << std::endl;
    }
    
    boost::filesystem::path dir3("markdowns/"+to_string(NNODES)+"-nodes/solutions-"+id+"/"+to_string(cost)+"-cost/");

    if(!(exists(dir3))){
        std::cout<<"Doesn't Exists"<<std::endl;

        if (create_directory(dir3))
            std::cout << "....Successfully Created !" << std::endl;
    }


    markdown.open("markdowns/"+to_string(NNODES)+"-nodes/solutions-"+id+"/"+to_string(cost)+"-cost/"+to_string(n-batchSize+2)+"-"+to_string(n+1)+".md"); // opens the file
    if( !markdown ) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    } else {
        cout << "Successfully Opened File for " << n-batchSize+2 << "-" << n+1 << " rounds" << endl;
    }
    markdown << "# the graphs for number of " << NNODES << " nodes from " << n-batchSize+2 << " to " << n+1 << endl;
    for (int i = 0; i < batchSize; i++) {
        markdown << "## Graph " << n-batchSize+2+i << " - appeared " << GraphList[i].appeared << "times" << endl;
        markdown << "___" << endl;
        markdown << "```graphviz"<< endl;
        write_graphviz(markdown, GraphList[i].g, vertexWriter, edgeWriter);
        markdown << "```"<< endl;
        markdown << "___" << endl;
    }
    markdown.close();
}

int main(int argc, char const *argv[])
{
    // program to take each skeleton and expand it to find if it exists a solution to the feistel graph problem (is any nodes can acess to any node in a fixed cost)
    
    if (argc < 3) {
        cout << "Usage: ./solve <number-of-pairs> <cost> (facultative):<id-part-file>" << endl;
        return 1;
    }
    

    NNODES = atoi(argv[1]);
    cost = atoi(argv[2]);

    if (cost <= 1){
        cout << "Error: cost must be greater than 1" << endl;
        return 1;
    }
    
    int batchSizeDisplay = 100;
    ifstream in;
    if (argc == 3){
        in.open("graphGen/graphs-"+to_string(NNODES)+".txt");
    } else {
        in.open("graphGen/graphs-"+to_string(argv[3])+".txt");
    }
    
    //ifstream lconfs("partitions/part-"+to_string(NNODES)+".txt");
    archive::text_iarchive ia(in);

    vector<Graph> skeletons;

    
    // ceci est un test avec vim  
    while (!in.eof()) {
        Graph gp;
        ia>>gp;

        skeletons.push_back(gp);

    }

    vector<vector<Record>> threadSols = vector<vector<Record>>();
    for (int i = 0; i < THREADS;i++){
        threadSols.push_back(vector<Record>());
    }
    
    #pragma omp parallel for num_threads(THREADS)
    for (int i=0;i<skeletons.size();i++){
        Graph g = skeletons[i];
        cout << "graph number " << i+1 << endl;
        nextPath(g,threadSols[omp_get_thread_num()]);
    }

    vector<Record> sols = vector<Record>();
    for (int i = 0; i < THREADS;i++){
        for (int j = 0; j < threadSols[i].size();j++){
            addToList(threadSols[i][j].g,threadSols[i][j].appeared,sols);
        }
    }

    if (sols.size() > 0){
        int c = 0;
        for (int n=0;n<sols.size();n++){
            if (c%batchSizeDisplay == batchSizeDisplay-1) {
                vector<Record> soltemp = vector<Record>(sols.begin()+c-batchSizeDisplay+1,sols.begin()+c);
                if (argc == 3){
                    displayMD(soltemp[0].g,soltemp,batchSizeDisplay,n,"classic");
                } else {
                    displayMD(soltemp[0].g,soltemp,batchSizeDisplay,n,to_string(argv[3]));
                }
            }
            c++;
        }
        vector<Record> soltemp = vector<Record>(sols.begin()+c-c%batchSizeDisplay,sols.end());
        if (argc == 3){
            displayMD(soltemp[0].g,soltemp,c%batchSizeDisplay,c-1,"classic");
        } else {
            displayMD(soltemp[0].g,soltemp,c%batchSizeDisplay,c-1,to_string(argv[3]));
        }
    } else {
        cout << "no solution found" << endl;
        // do something to indicate that no solutions were found
    }

    in.close();
    return 0;
}
