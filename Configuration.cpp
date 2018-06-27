
////////////////////////////////////
// (C)2007-2008 Coolsoft Company. //
// All rights reserved.           //
// http://www.coolsoft-sd.com     //
// Licence: licence.txt           //
////////////////////////////////////

#include "stdafx.h"
#include "Configuration.h"
#include "Professor.h"
#include "StudentsGroup.h"
#include "Course.h"
#include "Room.h"
#include "CourseClass.h"
#include <hash_map>
#include <string>
Configuration Configuration::_instance;
int lab_count=1;

// Frees used resources
Configuration::~Configuration()
{
	for (hash_map<int, Professor*>::iterator it = _professors.begin(); it != _professors.end(); it++)
		delete (*it).second;

	for (hash_map<int, StudentsGroup*>::iterator it = _studentGroups.begin(); it != _studentGroups.end(); it++)
		delete (*it).second;

	for (hash_map<int, Course*>::iterator it = _courses.begin(); it != _courses.end(); it++)
		delete (*it).second;

	for (hash_map<int, Room*>::iterator it = _rooms.begin(); it != _rooms.end(); it++)
		delete (*it).second;

	for (list<CourseClass*>::iterator it = _courseClasses.begin(); it != _courseClasses.end(); it++)
		delete *it;
}

// Parse file and store parsed object
void Configuration::ParseFile(char* fileName)
{
	// clear previously parsed objects
	_professors.clear();
	_studentGroups.clear();
	_courses.clear();
	_rooms.clear();
	_courseClasses.clear();

	Room::RestartIDs();

	// open file
	ifstream input(fileName);
	// 한글도 입력 가능
	_tsetlocale(LC_ALL, _T("kor"));

	string line;
	//파일이 열리고 끝나기 전까지
	while (input.is_open() && !input.eof())
	{
		// get lines until start of new object is not found
		// 처음부터 끝까지 빈칸 신경쓰지 않고 모든 값을 다 넣어줌
		getline(input, line);
		// 빈칸 제거
		TrimString(line);
		
		//input >> line; 이걸로 대체 가능

		// get type of object, parse obect and store it

		if (line.compare("#prof") == 0)
		{
			Professor* p = ParseProfessor(input);
			if (p)
				_professors.insert(pair<int, Professor*>(p->GetId(), p));
		}
	
		else if (line.compare("#course") == 0)
		{
			Course* c = ParseCourse(input);
			
			if (c)
				_courses.insert(pair<int, Course*>(c->GetId(), c));
		}
		else if (line.compare("#room") == 0)
		{
			Room* r = ParseRoom(input);
			if (r)
				_rooms.insert(pair<int, Room*>(r->GetId(), r));
		}
		else if (line.compare("#class") == 0)
		{
			CourseClass* c = ParseCourseClass(input);
			if (c)
				_courseClasses.push_back(c);
		}
	}

	input.close();

	_isEmpty = false;
}

// Reads professor's data from config file, makes object and returns pointer to it
// Returns NULL if method cannot parse configuration data
Professor* Configuration::ParseProfessor(ifstream& file)
{
	int id = 0;
	string name;

	while (!file.eof())
	{
		string key, value;

		// get key - value pair
		if (!GetConfigBlockLine(file, key, value))
			break;

		// get value of key
		if (key.compare("id") == 0)
			id = atoi(value.c_str());
		else if (key.compare("name") == 0)
			name = value;
	}

	// make object and return pointer to it
	return id == 0 ? NULL : new Professor(id, name);
}

// Reads professor's data from config file, makes object and returns pointer to it
// Returns NULL if method cannot parse configuration data
StudentsGroup* Configuration::ParseStudentsGroup(ifstream& file)
{
	int id = 0, number = 0;
	string name;

	while (!file.eof())
	{
		string key, value;

		// get key - value pair
		if (!GetConfigBlockLine(file, key, value))
			break;

		// get value of key
		if (key.compare("id") == 0)
			id = atoi(value.c_str());
		else if (key.compare("name") == 0)
			name = value;
		else if (key.compare("size") == 0)
			number = atoi(value.c_str());
	}

	// make object and return pointer to it
	return id == 0 ? NULL : new StudentsGroup(id, name, number);
}

// Reads course's data from config file, makes object and returns pointer to it
// Returns NULL if method cannot parse configuration data
Course* Configuration::ParseCourse(ifstream& file)
{
	int id = 0;
	string name;

	while (!file.eof())
	{
		string key, value;

		// get key - value pair
		if (!GetConfigBlockLine(file, key, value))
			break;

		// get value of key
		if (key.compare("id") == 0)
			id = atoi(value.c_str());
		else if (key.compare("name") == 0)
			name = value;
	}

	// make object and return pointer to it
	return id == 0 ? NULL : new Course(id, name);
}

// Reads rooms's data from config file, makes object and returns pointer to it
// Returns NULL if method cannot parse configuration data
Room* Configuration::ParseRoom(ifstream& file)
{
	int number = 0;
	bool lab = false;
	string name;

	while (!file.eof())
	{
		string key, value;

		// get key - value pair
		if (!GetConfigBlockLine(file, key, value))
			break;

		// get value of key
		if (key.compare("name") == 0)
			name = value;
		else if (key.compare("lab") == 0)
			lab = value.compare("true") == 0;
		else if (key.compare("size") == 0)
			number = atoi(value.c_str());
	
	}

	// make object and return pointer to it
	return number == 0 ? NULL : new Room(name, lab, number);
}

// Reads class' data from config file, makes object and returns pointer to it
// Returns NULL if method cannot parse configuration data
CourseClass* Configuration::ParseCourseClass(ifstream& file)
{
	int pid = 0, cid = 0, dur = 1,division=0;
	bool lab = false;
	bool lab_H = false;
	bool fix = false;
	string fixed,temp;
	int fix_time=0;
	list<StudentsGroup*> groups;
	
	while (!file.eof())
	{
		string key, value;
		
		// get key - value pair
		if (!GetConfigBlockLine(file, key, value))
			break;

		// get value of key
		if (key.compare("professor") == 0)
			pid = atoi(value.c_str());
		else if (key.compare("course") == 0)
			cid = atoi(value.c_str());
		else if (key.compare("lab") == 0)
		{
			lab = value.compare("true") == 0;
		}
		else if (key.compare("lab_H") == 0)
		{
			lab = value.compare("true") == 0;
			lab_H = true;
		}
		else if (key.compare("duration") == 0)
			dur = atoi(value.c_str());
		else if (key.compare("group") == 0)
		{
			StudentsGroup* g = GetStudentsGroupById(atoi(value.c_str()));
			if (g)
				groups.push_back(g);
		}
		else if (key.compare("division") == 0)
			division = atoi(value.c_str());
		else if (key.compare("fixed") == 0)
		{
			fix = true;
			fixed = value.c_str();
			
			temp.push_back(fixed[2]);
			if(fixed.size()>3)
				temp.push_back(fixed[3]);
			fix_time = atoi(temp.c_str());
		}
	}
	
	// get professor who teaches class and course to which this class belongs
	Professor* p = GetProfessorById(pid);
	Course* c = GetCourseById(cid);
	//if( temp !=NULL)
		
	//c->_name += fixed;
	CourseClass* cc = new CourseClass(p, c, groups, lab, dur, division);
	cc->_ClassCode = NULL;

	
	// does professor and class exists
	if (!c || !p)
		return NULL;
	
	
		// 3시간 이상인 수업은 2개로 만든다.//컴퓨터공학종합설계1,2가 아니면
		if (dur >= 3)
		{
			//실습이 있는 수업은 3학점이라도 2시간씩 수업함
			if (lab)
				dur = 2;
			// make object and return pointer to it
			if(lab_H == true && fix == false)
				cc = new CourseClass(p, c, groups, false , dur,division);
			else
				cc = new CourseClass(p, c, groups, lab, dur, division);
			
			// 3시간 이상인 수업은 2개로 만든다.
			CourseClass* cc1 = new CourseClass(p, c, groups, lab, dur, division);
			_courseClasses.push_back(cc1);
	
			// 서로 연결
			cc1->_ClassCode = cc;
			cc->_ClassCode = cc1;

			if (fix == true)
			{
				cc1->_fix = true;
				cc->_fix = true;
				cc->room_index = lab_count;
				cc1->room_index = lab_count++;
				if (fixed[0] == 'F')
				{
					cc->day_index = 4;
					cc1->day_index = 2;
				}
				else if (fixed[0] == 'T')
				{
					cc->day_index = 3;
					cc1->day_index = 1;
				}
				else if (fixed[0] == 'W')
				{
					cc->day_index = 2;
					cc1->day_index = 0;
				}
				else if (fixed[0] == 't')
				{
					cc->day_index = 1;
					cc1->day_index = 3;
				}
				else if (fixed[0] == 'M')
				{
					cc->day_index = 0;
					cc1->day_index = 2;
				}

				if (fix_time != 0)
				{
					cc->time_index = fix_time;
					cc1->time_index = fix_time;
				}
			}
		}
	
		
	return cc;
}

// Reads one line (key - value pair) from configuration file
bool Configuration::GetConfigBlockLine(ifstream& file, string& key, string& value)
{
	string line;

	// end of file
	while (!file.eof())
	{
		// read line from config file
		getline(file, line);
		TrimString(line);

		// end of object's data 
		if (line.compare("#end") == 0)
			return false;

		size_t p = line.find('=');
		if (p != line.npos)
		{
			// key
			// =를 찾기 전까지의 길이가 p
			key = line.substr(0, p);
			//빈칸 제거
			TrimString(key);

			// value
			value = line.substr(p + 1, line.length());
			// 빈칸제거
			TrimString(value);

			// key - value pair read successfully
			return true;
		}
	}

	// error
	return false;
}

// Removes blank characters from beginning and end of string
string& Configuration::TrimString(string& str)
{
	string::iterator it;
	for (it = str.begin(); it != str.end() && isspace(*it); it++)
		;
	str.erase(str.begin(), it);

	string::reverse_iterator rit;
	for (rit = str.rbegin(); rit != str.rend() && isspace(*rit); rit++)
		;
	str.erase(str.begin() + (str.rend() - rit), str.end());

	return str;
}

