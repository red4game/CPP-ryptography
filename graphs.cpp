#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp> // Include for boost::split
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/filesystem.hpp>
#include <bits/stdc++.h>



using namespace boost;
using namespace std;
int NNODES;
struct VertexData {
    int idVertex;
    int colorVertex;
    bool HeadChain;
    bool TailChain;
};

struct EdgeData {
    int colorEdge;
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

typedef adjacency_list<vecS, vecS, bidirectionalS,VertexData,EdgeData> Graph;

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

int sumPairOfVector(pair<vector<int>,vector<int>> & pair)
{
    int sum = 0;
    for (auto elem: pair.first)
    {
        sum += elem;
    }
    for (auto elem: pair.second)
    {
        sum += elem;
    }
    return sum;
}


Graph createGraph(pair<vector<int>,vector<int>> partition){
    //instantiate graph with pairs pairs of vertices



    
    if (sumPairOfVector(partition) != NNODES) {
        throw "Error: wrong partition";
    }

    Graph g(2*NNODES);

    
    // instanciate the vertices
    for (int i = 0; i < 2*NNODES; i++) {
        g[i].idVertex = i;
        g[i].colorVertex = i%2;
        g[i].HeadChain = false;
        g[i].TailChain = false;
    }

    // create epsilon-transitions for each pair from green to red
    for (int i = 0; i < 2*NNODES; i+=2) {
        auto e = add_edge(i, i+1, g).first;
        g[e].colorEdge = 0;
    }

    //making the cycles in the graph
    // warning : i don't know why but it is buging when i have 0 cycles in the graph
    int bound = 0;
    for (int cycles : partition.first) {
        if (cycles == 0) {
            break;
        }
        for (int i = bound+1; i < bound+2*cycles-1; i+=2) {
            
            auto e = add_edge(i, i+1, g).first;
            g[e].colorEdge = 1;
        }
        auto e = add_edge(bound+2*cycles-1, bound, g).first;
        g[e].colorEdge = 1;
        
        bound += 2*cycles;
    }

    //making the chains in the graph
    for (int chains : partition.second) {
        if (chains == 0) {
            break;
        }
        g[bound].HeadChain = true;
        for (int i = bound+1; i < bound+2*chains-1; i+=2) {
            auto e = add_edge(i, i+1, g).first;
            g[e].colorEdge = 1;
        }
        g[bound+2*chains-1].TailChain = true;
        
        bound += 2*chains;
    }
    return g;

}

pair<vector<int>,vector<int>> PartitionParser(string line){
    vector<string> words;
    split(words, line, boost::is_any_of("|"));
    
    vector<string> tempLeft;
    vector<string> tempRight;

    vector<int> left;
    vector<int> right;
    split(tempLeft, words[0], boost::is_any_of(","));
    split(tempRight, words[1], boost::is_any_of(","));

    for (string elem : tempLeft) {
        left.push_back(stoi(elem));
    }
    for (string elem : tempRight) {
        right.push_back(stoi(elem));
    }
    

    return make_pair(left,right);
}
void printPair(pair<vector<int>,vector<int>> & pair)
{
    cout << "Left: ";
    for (auto elem : pair.first) {
        cout << elem << " ";
    }
    cout << endl;
    cout << "Right: ";
    for (auto elem : pair.second) {
        cout << elem << " ";
    }
    cout << endl;
}
void displayMD(Graph &g,vector<Graph> &GraphList,vector<string> configLists,int batchSize,int n) {
    VertexWriter<Graph> vertexWriter(g);
    EdgeWriter<Graph> edgeWriter(g);
    ofstream markdown;

    boost::filesystem::path dir("markdowns/"+to_string(NNODES)+"-nodes/");

    if(!(exists(dir))){
        std::cout<<"Doesn't Exists"<<std::endl;

        if (create_directory(dir))
            std::cout << "....Successfully Created !" << std::endl;
    }

    boost::filesystem::path dir2("markdowns/"+to_string(NNODES)+"-nodes/partitions/");

    if(!(exists(dir2))){
        std::cout<<"Doesn't Exists"<<std::endl;

        if (create_directory(dir2))
            std::cout << "....Successfully Created !" << std::endl;
    }


    markdown.open("markdowns/"+to_string(NNODES)+"-nodes/partitions/"+to_string(n-batchSize+2)+"-"+to_string(n+1)+".md"); // opens the file
    if( !markdown ) { // file couldn't be opened
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    } else {
        cout << "Successfully Opened File for " << n-batchSize+2 << "-" << n+1 << " rounds" << endl;
    }
    markdown << "# the graphs for number of " << NNODES << " nodes from " << n-batchSize+2 << " to " << n+1 << endl;
    for (int i = 0; i < batchSize; i++) {
        markdown << "## Graph " << n-batchSize+2+i << " - list de compréhension : " << configLists[i] << endl;
        markdown << "___" << endl;
        markdown << "```graphviz"<< endl;
        write_graphviz(markdown, GraphList[i], vertexWriter, edgeWriter);
        markdown << "```"<< endl;
        markdown << "___" << endl;
    }
    markdown.close();
}

int main(int argc, char const *argv[])
{
    //program that take a file with repartition sof chains and cycle within combinations of all partitions in the universe to create an archive of skeleton graphs with their corresponding list of configurations
    if (argc < 2) {
        cout << "Usage: ./graphs <number of nodes> (facultative):<id-part-file>" << endl;
        return 1;
    }
    NNODES = stoi(argv[1]);
    
    Graph g;

    
    VertexWriter<Graph> vertexWriter(g);
	EdgeWriter<Graph> edgeWriter(g);
    ifstream partitions;
    if (argc == 2){
        partitions.open("partitions/part-"+to_string(NNODES)+".txt");
    } else {
        partitions.open("partitions/part-"+to_string(argv[2])+".txt");
    }
    if (!partitions.is_open()) {
        cerr << "Could not open the file - '"
             << "partitions/part-"+to_string(NNODES)+".txt" << "'" << endl;
        return EXIT_FAILURE;
    }
    ofstream out;
    if (argc == 2){
        out.open("graphGen/graphs-"+to_string(NNODES)+".txt");
    } else {
        out.open("graphGen/graphs-"+to_string(argv[2])+".txt");
    }
    archive::text_oarchive oa(out);
    string line;
    int n = 0;
    while (getline(partitions, line)){
        pair<vector<int>,vector<int>> partition = PartitionParser(line);
        //printPair(partition);
        g = createGraph(partition);
        oa << g;
    }
    out.close();
    
    //partie permettant de lire l'archive et de créer les graphes dans un markdown (partie en guise de code exemple)
    
    /*
    int batchSizeDisplay = 100;
    vector<Graph> glist(batchSizeDisplay);
    vector<string> configList(batchSizeDisplay);

    string config;
    ifstream in;
    ifstream lconfs;
    in.open("graphGen/graphs-"+to_string(NNODES)+".txt");
    lconfs.open("partitions/part-"+to_string(NNODES)+".txt");
    archive::text_iarchive ia(in);
    while (!in.eof()) {
        Graph gp;
        ia>>gp;
        lconfs>>config;
        glist[n%batchSizeDisplay] = gp;
        configList[n%batchSizeDisplay] = config;
        if (n%batchSizeDisplay == batchSizeDisplay-1) {
          displayMD(gp,glist,configList,batchSizeDisplay,n);
        }
        n++;
        
    }
    displayMD(g,glist,configList,(n%batchSizeDisplay),n-1);
    in.close();
    //write_graphviz(cout, gp, vertexWriter, edgeWriter);
    */
    return 0;
}
