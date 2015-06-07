/*****************************************************************************
 * Copyright (C) 2014 Visualink
 *
 * Authors: Adrien Maglo <adrien@visualink.io>
 *
 * This file is part of Pastec.
 *
 * Pastec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pastec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Pastec.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include <iostream>
#include <signal.h>

#include <orb/orbfeatureextractor.h>
#include <orb/orbsearcher.h>
#include <orb/orbwordindex.h>
#include "messages.h"
#include <opencv2/opencv.hpp>

using namespace std;

std::string execCmd(const char* cmd) {
	FILE* pipe = _popen(cmd, "r");
	if (!pipe) return "";
	char buffer[128];
	std::string result = "";
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);
	return result;
}

vector<string> getFilesFromDir(const string dirPath)
{
	CV_Assert(dirPath.size() > 0);
	const string cmd = "dir /B /S /A:-D " + dirPath;
	const string content = execCmd(cmd.c_str());
	vector<string > result;
	stringstream ss(content);
	string line;
	while (getline(ss,line))
	{
		result.push_back(line);
	}
	return result;
}

void generateHtml(const string srcImgPath, vector<string> matchedImgsPath)
{
	stringstream ss;
	ss << "<img src=" << srcImgPath << "><br/><hr/>";
	for (size_t i = 0; i < matchedImgsPath.size();i++)
	{
		ss << "top " << i << " <br/>";
		ss << "<img src=" << matchedImgsPath[i] << "><br/>";
	}
	ofstream ofs("debug.html");
	ofs << ss.str();
	ofs.close();
	system("debug.html");
}
int main(int argc, char** argv)
{
    cout << "Pastec Index v0.0.1" << endl;

    string visualWordPath("visualWordsORB.dat");
    string indexPath(DEFAULT_INDEX_PATH);

    Index *index = new ORBIndex(indexPath);
	ORBWordIndex *wordIndex = new ORBWordIndex(visualWordPath);
	FeatureExtractor *featureExtractor = new ORBFeatureExtractor((ORBIndex *)index, wordIndex);
	Searcher *imageSearcher = new ORBSearcher((ORBIndex *)index, wordIndex);

	map<int, string> imageMaps;

	//step1 : add images to databse
	if (true)
	{
		const vector<string> trainfiles = getFilesFromDir(R"(D:\opensource\pastec\images\train)");
		int index = 0;
		for (const auto file : trainfiles)
		{
			Mat src = imread(file);
			if (src.empty())
			{
				continue;
			}
			ifstream ifs(file,ios::binary | ios::in);
			vector<char> buffer((std::istreambuf_iterator<char>(ifs)),(std::istreambuf_iterator<char>()));
			auto insertFunc = [](FeatureExtractor *featureExtractor, const int index, vector<char>& buffer)->bool{
					__try
					{
						int i_ret = featureExtractor->processNewImage(index, buffer.size(), &(buffer[0]));
						if (i_ret != IMAGE_ADDED)
						{							
							return false;
						}
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{						
						cerr << "some exception." << endl;
						return false;
					}
					return true;
				};
			if (insertFunc(featureExtractor, index, buffer))
			{
				imageMaps.insert(make_pair(index, file));
			}
			else
			{
				cerr << "processNewImage failed , path = " << file << endl;
				continue;
			}		
			index++;
		}		
	}
	//step2 : save index
	if (false)
	{
		int i_ret = index->write(indexPath);
	}
	//step2 : search image
	if (true)
	{
		const vector<string> testfiles = getFilesFromDir(R"(D:\opensource\pastec\images\test)");
		for (const auto file : testfiles)
		{
			Mat src = imread(file);
			if (src.empty())
			{
				continue;
			}
			SearchRequest req;
			ifstream ifs(file, ios::binary | ios::in);
			std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), std::back_inserter(req.imageData));
			auto searchFunc = [](Searcher *imageSearcher,SearchRequest& req)->int{
				int i_ret = -1;
				__try
				{
					i_ret = imageSearcher->searchImage(req);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{

				}
				return i_ret;
			};
			const int i_ret = searchFunc(imageSearcher, req);
			cout << "-------------------------------" << endl << endl;
			cout << "current file is " << file <<endl;
			cout << "results : " << endl;
			if (i_ret == SEARCH_RESULTS)
			{
				vector<string> matchedImgsPath;
				for (size_t i = 0; i < req.results.size();i++)
				{
					const int imgIdx = req.results[i];
					cout << "option result : index="<<imgIdx<<",path="<<imageMaps[imgIdx] << endl;
					matchedImgsPath.push_back(imageMaps[imgIdx]);
				}
				generateHtml(file, matchedImgsPath);
			}
			else
			{
				cout << "not found!!!" << endl;
			}
			cout << "-------------------------------" << endl<<endl;
			imshow("gray iamge for search", src);
			waitKey(0);
		}		
	}
	//step3 : other funtions
	if (false)
	{
		{
			int i_ret = index->removeImage(0);
		}
		{
			int i_ret;
			i_ret = index->load("index_path");
			i_ret = index->write("index_path");
			i_ret = index->clear();
		}
	}

	delete (ORBSearcher *)imageSearcher;
	delete (ORBFeatureExtractor *)featureExtractor;
    delete (ORBIndex *)index;

    return 0;
}
