#include <iostream>
#include <curl/curl.h>
#include <string>
#include <queue>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <thread>
#include <mutex>

using namespace std;
mutex m1;
queue<string> visitURLS;
queue<string> seenURLS;
queue<string> tempURLS;

void FilterLinks(string& link)
{
    int lenhyper = link.length();
    if (link[lenhyper - 1] == 'g' && link[lenhyper - 2] == 'n' && link[lenhyper - 3] == 'p')
    {
        link.clear();
    }
    if (link[lenhyper - 1] == 'g' && link[lenhyper - 2] == 'p' && link[lenhyper - 3] == 'j')
    {
        link.clear();
    }
    if (link[lenhyper - 1] == ',' || link[lenhyper - 1] == 'w' || link[lenhyper - 1] == '<' || link[lenhyper - 1] == '@' || link[lenhyper - 1] == '>' || (link[lenhyper - 1] == '/' && link[lenhyper - 2] == '*') || (link[lenhyper - 1] == ' ') || link[lenhyper - 1] == ';' || link[lenhyper - 1] == '=')
    {
        link.clear();
    }

    if (link.length() <= 8)
    {
        link.clear();


    }
    if (link[8] == 'c' && link[9] == 'd' && link[10] == 'n')
    {
        link.clear();
    }
}


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void showq()
{
    while (!visitURLS.empty())
    {
        cout << '\t' << visitURLS.front();
        visitURLS.pop();
    }
    cout << endl;
}


void getSeeds(string url)
{

    cout << endl;
    cout << "Url is : " << url << endl;
    m1.lock();
    //curl objects
    CURL* curl;
    CURLcode result;
    string readBuffer;

    //curl handle
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        result = curl_easy_perform(curl);
        if (result != CURLE_OK)
        {
            cout << "curl easy perform() failed" << endl;
            cout << curl_easy_strerror(result) << endl;
        }
        curl_easy_cleanup(curl);
        m1.unlock();
        //cout << readBuffer << endl;
        cout << "Length of readBuffer is : " << readBuffer.length() << endl;

    }
    else
    {
        cout << "Curl fail" << endl;
    }

    cout << "Finding seeds.." << endl;
    string hyperLink;

    for (int i = 0; i < readBuffer.length(); i++)
    {
        //if(readBuffer[i] == '"'){

        if (readBuffer[i] == 'h') {
            if (readBuffer[i + 1] == 't') {
                if (readBuffer[i + 2] == 't') {
                    if (readBuffer[i + 3] == 'p') {
                        if (readBuffer[i + 4] == 's') {
                            for (int j = i; j < readBuffer.length(); j++) {
                                if (readBuffer[j] == '"' || readBuffer[j] == '\'')
                                {
                                    break;
                                }
                                else {
                                    hyperLink += readBuffer[j];
                                }
                            }
                        }
                    }
                }
            }
        }

        FilterLinks(hyperLink);
        //}

        if (!hyperLink.empty()) {
            cout << "Hyper Link : " << hyperLink << endl;
            visitURLS.push(hyperLink);
        }
        hyperLink.clear();
    }
    curl_global_cleanup();
}

void sendURL(string url)
{
    seenURLS.push(url);

    getSeeds(url);

    while (!visitURLS.empty())
    {
        //cout << "Displaying QUEUE" << endl;
        //showq();
        cout << endl;
        while (!seenURLS.empty())
        {
            if (seenURLS.front() != visitURLS.front())
            {
                string temp;
                temp = seenURLS.front();
                seenURLS.pop();
                tempURLS.push(temp);
            }
            else if (seenURLS.front() == visitURLS.front())
            {
                visitURLS.pop();
                cout << "Removing duplicate" << endl;
            }
        }
        while (!tempURLS.empty())
        {
            seenURLS.push(tempURLS.front());
            tempURLS.pop();
        }
        url = visitURLS.front();
        seenURLS.push(url);
        cout << "New URL is : " << url << endl;
        visitURLS.pop();
        cout << "Next URL in queue : " << visitURLS.front();
        getSeeds(url);
    }

    if (visitURLS.empty())
    {
        cout << "Queue Empty" << endl;
    }
}

int main(void)
{
    thread t1(sendURL, "https://www.stackoverflow.com");
    t1.join();
    thread t2(sendURL, "https://www.geeksforgeeks.org");
    t2.join();

    return 0;
}