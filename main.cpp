#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/filesystem.hpp>
#include <bits/stdc++.h>


using namespace boost;
using namespace std;
                  
#define NNODES 20


int fact(int n) {
   if ((n==0)||(n==1))
   return 1;
   else
   return n*fact(n-1);
}

struct VertexData {
    int idVertex;
    int colorVertex;
};

struct EdgeData {
    int idEdge;
    int colorEdge;
};


typedef adjacency_list<vecS, vecS, directedS,VertexData,EdgeData> Graph;

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

void displayMD(Graph &g,vector<Graph> &GraphList,vector<vector<int>> configLists,int batchSize,int n,int cost) {
  VertexWriter<Graph> vertexWriter(g);
	EdgeWriter<Graph> edgeWriter(g);
  ofstream markdown;

  boost::filesystem::path dir("markdowns/"+to_string(NNODES)+"-nodes/"+to_string(cost)+"-cost/");

  if(!(exists(dir))){
      std::cout<<"Doesn't Exists"<<std::endl;

      if (create_directory(dir))
          std::cout << "....Successfully Created !" << std::endl;
  }

  markdown.open("markdowns/"+to_string(NNODES)+"-nodes/"+to_string(cost)+"-cost/"+to_string(n-batchSize+2)+"-"+to_string(n+1)+".md"); // opens the file
  if( !markdown ) { // file couldn't be opened
      cerr << "Error: file could not be opened" << endl;
      exit(1);
  } else {
    cout << "Successfully Opened File for " << n-batchSize+2 << "-" << n+1 << " rounds" << endl;
  }
  markdown << "# the graphs for number of " << NNODES << " nodes from " << n-batchSize+2 << " to " << n+1 << " with a cost for TVS of " << cost << endl;
  for (int i = 0; i < batchSize; i++) {
    std::stringstream ss;
    for (auto it = configLists[i].begin(); it != configLists[i].end(); it++)    {
        if (it != configLists[i].begin()) {
            ss << " ";
        }
        ss << (*it);
    }
    markdown << "## Graph " << n-batchSize+2+i << " - list de compréhension : " << ss.str() << endl;
    markdown << "___" << endl;
    markdown << "```graphviz"<< endl;
    write_graphviz(markdown, GraphList[i], vertexWriter, edgeWriter);
    markdown << "```"<< endl;
    markdown << "___" << endl;
  }
  markdown.close();
}

bool TVSB(Graph &g,int u,int t, int n) {
  if (u==t && n == 0)
    return true;
  else {
    if (n>0){
      for (auto ed : make_iterator_range(boost::out_edges(u, g))){
        if (TVSB(g,target(ed, g),t,n-g[ed].colorEdge) == true)
          return true;
      }
      return false;
    } else {
      return false;
    }
  }
}

bool verifyConfig(Graph &g,int cost) {
  bool verified;
  verified = true;
  for (auto u : boost::make_iterator_range(vertices(g))) {
    for (auto v : boost::make_iterator_range(vertices(g))) {
      if (u != v) {
        if (TVSB(g,u,v,cost) == false) {
          verified = false;
          break;
        }
      }
    }
  }
  return verified;
}

int resolveUniverse(Graph &g,int batchSizeDisplay,int cost) {
  vector<int> a(NNODES);
  for (int i =0;i<NNODES;i++){
    a[i] = (i+1)%NNODES;
  }
  vector<Graph> glist(batchSizeDisplay);
  vector<vector<int>> listConfigs(batchSizeDisplay);
  int n = 0;
  bool viable = true;
  cout << "Démarrage du  programme de recherche de génération de l'univers et de résolution des solutions pour " << NNODES << " noeuds - avec un TVS de coût " << cost << endl;
  
  do{
    // need optimizations on heuristic part --> maybe kruskal's algorithm  or other algorithm to find the linkability of all nodes
    // this will be here that we will do the heuristic part with constraint programming
    Graph gp(g);
    for (int i = 0; i < NNODES; i++) {
      if (i%2 != 0 && i == a[i]) {  // here it's the condition of linkability and if it's false, then we put viable to false 
        viable = false;
        break;
      }

      auto e = add_edge(i,a[i],gp).first;
      gp[e].idEdge = NNODES+i;
      gp[e].colorEdge = 1;
    }
    if (viable) {
      // this is the part where we verify if this graph respect our
      if (verifyConfig(gp,cost)) {
        glist[n%batchSizeDisplay] = gp;
        listConfigs[n%batchSizeDisplay] = a;
        cout << "Progress of batch : " << (n%batchSizeDisplay) << "/" << batchSizeDisplay << endl;
        if (n%batchSizeDisplay == batchSizeDisplay-1) {
          displayMD(g,glist,listConfigs,batchSizeDisplay,n,cost);
        }
        n++;
      }
    }
    viable = true;

  } while(next_permutation(a.begin(), a.end())); //Generate next permutation till it is not lexicographically largest
  
  // do the part final when this is our last batch
  displayMD(g,glist,listConfigs,(n%batchSizeDisplay),n-1,cost);
  return n;
}



int main(int argc, char const *argv[])
{
  
  Graph g(NNODES);

  for (int i = 0; i < NNODES; i++) {
    g[i].idVertex = i;
    g[i].colorVertex = i%2;
  }

  for (int i = 0; i < NNODES; i+=2) {
    auto e = add_edge(i, i+1, g).first;
    g[e].idEdge = i;
    g[e].colorEdge = 0;
  }

  int batchSizeDisplay = 5;
  int cost = 11;
  resolveUniverse(g,batchSizeDisplay,cost);
  return 0;
}
