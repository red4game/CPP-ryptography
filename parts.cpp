#include <iostream> 
#include <omp.h>
#include <vector> 
#include <bits/stdc++.h>
#include <tuple>
#include <boost/filesystem.hpp>

#define MAX_THREADS 8

using namespace std;

bool is_readable( const string & file )  
{  
    ifstream fichier( file.c_str() );  
    return !fichier.fail();  
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
void printVector3D(vector<pair<vector<int>,vector<int>>> & matrix)
{
    for(auto row_obj : matrix)
    {
        
        int counter = 0;
        for (auto elem: row_obj.first)
        {  
            counter++;
            cout<<elem;
            if (counter<row_obj.first.size())
                cout<<",";
        }
        cout<<"|";
        counter=0;
        for (auto elem: row_obj.second)
        {  
            counter++;
            cout<<elem;
            if (counter<row_obj.second.size())
                cout<<",";
        }
        cout<<endl;
    }
}

void printVector2D(vector<vector<int>> & matrix)
{
    for(auto row_obj : matrix)
    {
        for (auto elem: row_obj)
        {
            cout<<elem<<", ";
        }
        cout<<endl;
    }
    cout<<endl;
}

void printVector(vector<int> & matrix)
{
    for(auto elem : matrix)
    {
        cout<<elem<<":";
    }
    cout<<endl;
}

void printVectorLast3D(vector<pair<vector<int>,vector<int>>> & matrix)
{
    auto row_obj = matrix.back();
        int counter = 0;
        for (auto elem: row_obj.first)
        {  
            counter++;
            cout<<elem;
            if (counter<row_obj.first.size())
                cout<<",";
        }
        cout<<"|";
        counter=0;
        for (auto elem: row_obj.second)
        {  
            counter++;
            cout<<elem;
            if (counter<row_obj.second.size())
                cout<<",";
        }
        cout<<endl;
}


void putIntoFile(ofstream &part,pair<vector<int>,vector<int>> & matrix)
{
    int counter = 0;
    for (auto elem: matrix.first)
    {
        counter++;
        part<<elem;
        if (counter<matrix.first.size())
            part<<",";
    }
    part<<"|";
    counter=0;
    for (auto elem: matrix.second)
    {
        counter++;
        part<<elem;
        if (counter<matrix.second.size())
            part<<",";
    }
    part<<endl;
}
    
void putIntoFile(ofstream &part,vector<pair<vector<int>,vector<int>>> & matrix,int num)
{
    part.open("partitions/part-"+to_string(num)+".txt",ios_base::app);
    for(auto row_obj : matrix)
    {
        putIntoFile(part,row_obj);
    }
    part.close();
}



void getPartitionsOther(const vector<int>& elements, vector<pair<vector<int>,vector<int>>> &fList,int size=2){
    fList.clear();
    vector<vector<int>> lists;
    vector<int> indexes(elements.size(), 0); // Allocate?
    lists.push_back(std::vector<int>());
    lists[0].insert(lists[0].end(), elements.begin(), elements.end());

    int counter = -1;

    for(;;){
        counter += 1;
        if(lists.size() <= size){
            if (lists.size() == 1)
                lists.push_back(vector<int>());

            lists[0].erase(std::remove(lists[0].begin(), lists[0].end(), 0), lists[0].end());
            lists[1].erase(std::remove(lists[1].begin(), lists[1].end(), 0), lists[1].end());

            if (lists[1].size()==0){
                lists[1].push_back(0);
            }
            pair<vector<int>,vector<int>> pairing = pair<vector<int>,vector<int>>(lists[0],lists[1]);
            pair<vector<int>,vector<int>> pairingInv = pair<vector<int>,vector<int>>(lists[1],lists[0]);
            fList.push_back(pairing);
            fList.push_back(pairingInv);
        }

        int i,index;
        bool obreak = false;
        for (i=indexes.size()-1;; --i) {
            if (i<=0){
                obreak = true;
                break;
            }
            index = indexes[i];
            lists[index].erase(lists[index].begin() + lists[index].size()-1);
            if (lists[index].size()>0)
                break;
            lists.erase(lists.begin() + index);
        }
        if(obreak) break;

        ++index;
        if (index >= lists.size())
            lists.push_back(vector<int>());
        for (;i<indexes.size();++i) {
            indexes[i]=index;
            lists[index].push_back(elements[i]);
            index=0;
        }

    }
}

void getPartitions(vector<int>& elements, vector<pair<vector<int>,vector<int>>> &fList){
    fList.clear();
    elements.erase(std::remove(elements.begin(), elements.end(), 0), elements.end());
    int count = pow(2, elements.size());
    #pragma omp parallel for num_threads(MAX_THREADS)
    for (int i = 0; i < count; i++) {
        // The inner for loop will run n times , 
        // As the maximum number of elements a set can have is n
        // This loop will generate a subset
        vector<int> subset;
        for (int j = 0; j < elements.size(); j++) {
            // This if condition will check if jth bit in 
            // binary representation of  i  is set or not
            // if the value of (i & (1 << j)) is not 0 , 
            // include arr[j] in the current subset
            // otherwise exclude arr[j]
            if ((i & (1 << j)) != 0)
                subset.push_back(elements[j]);
        }
        
        //printVector(subset);
        
        vector<int> left;
        vector<int> right;
        vector<int> modElements = vector<int>(elements);

        if (subset.size() == 0){
            left.push_back(0);
            right = modElements;
        } else if (subset.size() == elements.size()){
            left = modElements;
            right.push_back(0);
        } else {
            for (int j = 0; j < subset.size(); j++) {
                left.push_back(subset[j]);
                for (int h=0;h<modElements.size();h++){
                    if (modElements[h]==subset[j]){
                        modElements.erase(modElements.begin() + h);
                        break;
                    }
                }
            }
            right = modElements;
        }
        pair<vector<int>,vector<int>> pairing = pair<vector<int>,vector<int>>(left,right);
        //printPair(pairing);
        pair<vector<int>,vector<int>> pairingInv = pair<vector<int>,vector<int>>(right,left);
        
        #pragma omp critical // maybe put an array  of flist for each process and finally merge them
        {
            fList.push_back(pairing);
            fList.push_back(pairingInv);
        }

    }
}
/*
vector<vector<vector<int>>> limitSize(const vector<vector<vector<int>>>& fList, int size){
    vector<vector<vector<int>>> lList;
    for(auto& list : fList){
        if(list.size()<=size){
            lList.emplace_back(list);
        }
    }
    return lList;
}
*/

void findCombinationsUtil(vector<int> arr, int index,
                       int num, int reducedNum,ofstream &partition)
{
    // Base condition
    if (reducedNum < 0)
        return;
 
    // If combination is found, print it
    if (reducedNum == 0)
    {   
        vector<pair<vector<int>,vector<int>>> lLists;
        vector<int> r = vector<int>(arr);
        cout<<"Found: Partition"<<endl;
        printVector(r);
        r.erase(std::remove(r.begin(), r.end(), 0), r.end());
        getPartitions(r,lLists);
        sort(lLists.begin(), lLists.end());
        lLists.erase(std::unique(lLists.begin(), lLists.end()), lLists.end());
        putIntoFile(partition,lLists,num);
        
    }
 
    // Find the previous number stored in arr[]
    // It helps in maintaining increasing order
    int prev = (index == 0)? 1 : arr[index-1];
 
    // note loop starts from previous number
    // i.e. at array location index - 1
    
    for (int k = prev; k <= num ; k++)
    {
        // next element of array is k
        arr[index] = k;
        findCombinationsUtil(arr, index + 1, num, reducedNum - k,partition);
    }
}

void findCombinations(int n,vector<pair<vector<int>,vector<int>>> &ans,ofstream &partition)
{
    // array to store the combinations
    // It can contain max n elements*
    // don't know why need to allocate n+1 elements because index is offsetted by 1 in findCombinationsUtil() function
    // maybe because i replaced an array by a vector
    vector<int> arr(n+1);

    //find all combinations
    findCombinationsUtil(arr, 0, n, n,partition);
    
}

int main(int argc, char *argv[])
{   
    //program to create combinations of chains and cycle in all partitions of the universe
    if (argc!=2)
    {
        cout<<"Usage: ./parts <fibonacci boundary : int>"<<endl;
        return 0;
    }

    int n = atoi(argv[1]);
    vector<pair<vector<int>,vector<int>>> ans;
    ofstream partition;
    string file = "partitions/part-"+to_string(n)+".txt";
    if (is_readable(file)){
        cout<<"Careful the file" << file <<" already exists : do you want to remove it and recompute the file ?"<<endl;
        cout<<"y/n"<<endl;
        string c;
        cin>>c;
        if (c!="y" && c!="Y" && c!="yes" && c!="YES"){
            cout<<"Aborting"<<endl;
            
            return 0;
        } else {
            remove(file.c_str());
            cout << "File" << file << " removed successfully" << endl;
        }
    }
    findCombinations(n,ans,partition);
    //printVector3D(ans);
    return 0;
}