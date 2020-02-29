
#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>
#include <condition_variable>
#define PROGRAM_NAME 0

using namespace std;
mutex current_mutex;    // mutex variable used to control play class acess
condition_variable data_cond;
typedef struct playline // playline class, each line in the play correspond each playline class
{
    int linenumber;
    string rolename;
    string linetext;
} playline;

bool comparisonPlayLine(const playline& a, const playline& b) // compare function used to sort the playline in play class using the number of line
{
    return a.linenumber < b.linenumber;
}
class player
{
private:
    vector<playline> contents;
};
class play
{
private:
    string playName;
    vector<playline> contents;
    int counter;
public:
    explicit play(string name) : playName(move(name)),counter(0)
    {
    }
    play& operator<<(const playline& argument)
    {
        lock_guard<mutex> guard(current_mutex); // Every time we get access to the contents list, we need to acquire the mutex first.
        contents.push_back(argument);
        return *this;
    }
    string getName()
    {
        return playName;
    }
    void print(ostream& theStream)
    {
        lock_guard<mutex> guard(current_mutex);
        sort(contents.begin(), contents.end(), comparisonPlayLine); // sort the class member vector by using the number of line.
        string current;

        for (int i = 0; i < contents.size(); i++) // print the playline one by one.
        {
            if (i == 0)
            {
                theStream << contents[i].rolename << "." << endl;
                theStream << contents[i].linetext << endl;
                current = contents[i].rolename;
            }
            else
            {
                if (current != contents[i].rolename)
                {
                    theStream << endl;
                    theStream << contents[i].rolename << "." << endl;
                    current = contents[i].rolename;
                }
                theStream << contents[i].linetext << endl;
            }
        }
    }
    void recite(vector<playline>::iterator &contentIte)
    {
        unique_lock<mutex> lock(current_mutex);
        if (counter < (*contentIte).linenumber)
        {

        }
    }
};
string& trim(string& ss) // used to remove the beginning spaces from each given string
{
    string::size_type pos = ss.find_first_not_of(" \t");
    ss = ss.substr(pos);
    return ss;
}
bool is_number(string& s) // used to judge whether given string is Integer or not.
{
    string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}
void threadProcess(play& currentPlay, const string& rolename, ifstream& input)
{
    if (input.is_open())
        while (!input.eof())
        {

            playline newLine;
            string line;
            getline(input, line);
            if (line.empty() || line == " ") // Check given line is empty or only have space. if yes, continue.
            {
                continue;
            }
            string::size_type pos;
            pos = line.find(' ', 0); // split given line text from first space.
            string thisLineNumber, thisLineText;
            thisLineNumber = line.substr(0, pos);
            thisLineText = line.substr(pos + 1);
            if (thisLineText.empty() || !is_number(thisLineNumber)) // judge two part of string is legal or not. first part need to be a Integer, second part should not be equal to empty string
            {
                continue;
            }
            newLine.linenumber = stoi(thisLineNumber);
            newLine.linetext = trim(thisLineText);
            newLine.rolename = rolename;
            currentPlay << newLine; // Insert new playline to playline vector inside play class.
        }
    input.close();
}

int main(int argc, const char* argv[])
{
    // insert code here...
    string configName = argv[1];
    try
    {
        if (configName.empty())
        {
            cout << "usage: " << argv[PROGRAM_NAME] << " <configuration_file_name>" << endl;
            return 1;
        }
        ifstream myReadFile(configName);

        if (myReadFile.is_open())
        {
            string currentplayname;
            while (!myReadFile.eof()) // read first line of config file, use this content to be our current play name.
            {
                getline(myReadFile, currentplayname);
                if (currentplayname.empty())
                {
                    continue;
                }
                break;
            }
            play newPlay(currentplayname);
            while (!myReadFile.eof()) // read following lines, invoke each line with a separated thread and join them after invoked.
            {
                string currentLine;
                getline(myReadFile, currentLine);
                if (currentLine.empty())
                {
                    continue;
                }
                if (currentLine.find(' ') == string::npos)
                {
                    cout << "bad formated line" << endl;
                    continue;
                }

                stringstream ss(currentLine);
                string currentRole, currentFile;
                getline(ss, currentRole, ' ');
                getline(ss, currentFile, ' ');
                if (currentRole.empty() || currentFile.empty())
                {
                    cout << "bad formated line" << endl;
                    continue;
                }
                ifstream newRole;
                newRole.open(currentFile);
                thread current_thread(threadProcess, ref(newPlay), currentRole, ref(newRole));
                current_thread.join();
            }
            newPlay.print(cout); // using print function to all contents inside the play class in given format.
            myReadFile.close();
            return 0;
        }
        else
        {
            cout << "fail to open files" << endl;
            return 1;
        }
    }
    catch (exception & e)
    {
        cout << e.what() << endl;
        return 1;
    }
}
