#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>

using namespace std;
class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        //Have the vector of strings | get the size
        int n = strs.size();
        bool seen = false;
        int counter = 0;
        //Have a vector of hashmaps
        //For each string in strs,
        // Sort it
        // create a hashmap of the sorted char, index
        // Go through hashmaps in maps
        // If match, add to output at index
        // Otherwise, add to maps
        // Add to output
        vector<unordered_map<char, int>> maps;
        vector<vector<string>> output{n, vector<string>(1, "a")};
        vector<string> strscopy(strs);
        for(int i = 0; i < n; i++){
            seen = false;
            sort(strscopy[i].begin(), strscopy[i].end());
            unordered_map<char,int> temp;
            for(int j = 0; j < strscopy[i].size(); j++){
                temp[strscopy[i][j]] = j;
                cout << strscopy[i][j];
            }
            cout << endl;
            if(maps.size() == 0){
                maps.push_back(temp);
            }
            for(int j = 0; j < maps.size(); j++){
                if(maps[j] == temp){
                    seen = true;
                    if(j > 0){
                        output[j].push_back(strs[i]);

                    }else{
                        output[0][0] = (strs[i]);
                    }
                }
            }
            if(!seen){
                maps.push_back(temp);
            }
        }
        cout << "Size: "<<output.size()<<endl;
        cout << output[0].size();
        return output;
    }

};

int main() {
    try{
        vector<vector<string>> f;
        Solution t;
        vector<string> temp = {"eat","tea","tan","ate","nat","bat"};
        f = t.groupAnagrams(temp);
        //for(f.begin())
    } catch(...){
        cout << "Error" << endl;
    }
}
