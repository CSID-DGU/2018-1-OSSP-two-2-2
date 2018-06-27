
////////////////////////////////////
// (C)2007-2008 Coolsoft Company. //
// All rights reserved.           //
// http://www.coolsoft-sd.com     //
// Licence: licence.txt           //
////////////////////////////////////

#include <list>
#include "Professor.h"
#include "StudentsGroup.h"
#include "Course.h"

#pragma once

using namespace std;

// Stores data about single class
class CourseClass
{

private:

	// Profesor who teaches
	Professor* _professor;

	// Course to which class belongs
	Course* _course;

	// Student groups who attend class
	list<StudentsGroup*> _groups;

	// Number of seats (students) required in room
	int _numberOfSeats;

	// Initicates wheather class requires computers
	bool _requiresLab;

	// Duration of cla ss in hours
	int _duration;

	//분반 표시
	int _division;


public:

	//분할 수업인지 확인
	CourseClass * _ClassCode;

	//고정시간표인지 확인
	bool _fix = false;
	
	//해당 수업 교실, 시간
	int room_index;
	int day_index;
	int time_index;
	// Initializes class object
	CourseClass(Professor* professor, Course* course, const list<StudentsGroup*>& groups,
		bool requiresLab, int duration,int division);

	// Frees used memory
	~CourseClass();

	// Returns TRUE if another class has one or overlapping student groups.
	bool GroupsOverlap(const CourseClass& c) const;

	// Returns TRUE if another class has same professor.
	inline bool ProfessorOverlaps(const CourseClass& c) const { return *_professor == *c._professor; }

	// Return pointer to professor who teaches
	inline const Professor& GetProfessor() const { return *_professor; }

	// Return pointer to course to which class belongs
	inline const Course& GetCourse() const { return *_course; }

	// Returns reference to list of student groups who attend class
	inline const list<StudentsGroup*>& GetGroups() const { return _groups; }

	// Returns number of seats (students) required in room
	inline int GetNumberOfSeats() const { return _numberOfSeats; }

	// Returns TRUE if class requires computers in room.
	inline bool IsLabRequired() const { return _requiresLab; }
	// 분반번호 호출
	inline int GetDivision() const { return _division; }
	
	
	inline int GetDuration() const
	{ 
		
		if (_duration < 3)
		// 30분 단위로 바꾸었기 때문에 *2를 해줌
			return _duration * 2;
		// 3시간이 넘는 수업이면 수업을 둘로 쪼갠다
		else
			return _duration;
	}
	
};
