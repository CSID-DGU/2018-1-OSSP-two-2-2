
////////////////////////////////////
// (C)2007-2008 Coolsoft Company. //
// All rights reserved.           //
// http://www.coolsoft-sd.com     //
// Licence: licence.txt           //
////////////////////////////////////

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "ChildView.h"
#include "Configuration.h"
#include "Schedule.h"
#include "Room.h"
#include <cmath>

// Handles event that is raised when algorithm finds new best chromosome
void ScheduleObserver::NewBestChromosome(const Schedule& newChromosome)
{
	if (_window)
		_window->SetSchedule(&newChromosome);
}

// Handles event that is raised when state of execution of algorithm is changed
void ScheduleObserver::EvolutionStateChanged(AlgorithmState newState)
{
	if (_window)
		_window->SetNewState(newState);

	newState != AS_RUNNING ? ReleaseEvent() : BlockEvent();
}

// Initializes chromosomes with configuration block (setup of chromosome)
Schedule::Schedule(int numberOfCrossoverPoints, int mutationSize,
	int crossoverProbability, int mutationProbability) : _mutationSize(mutationSize),
	_numberOfCrossoverPoints(numberOfCrossoverPoints),
	_crossoverProbability(crossoverProbability),
	_mutationProbability(mutationProbability),
	_fitness(0) 
{
	// reserve space for time-space slots in chromosomes code
	//30분 단위로 변경 // 유전자의 길이(총 string의 길이)를 설정해줌
	//시간표를 하나의 string으로 설정
	
	_slots.resize(DAYS_NUM * DAY_HOURS * Configuration::GetInstance().GetNumberOfRooms());
	


	// reserve space for flags of class requirements
	// 각 CourseClasses에 Hard_Reqirements를 곱해줌으로써 만족시켜야할 총 기준을 설정해준다.
	// 예를 들어 CourseClass가 3개인 경우 hardRequirements를 곱하므로써 criteria의 길이를 설정할 수 있다.
	_criteria.resize(Configuration::GetInstance().GetNumberOfCourseClasses() * Hard_Requirements);
}

// Copy constructor
Schedule::Schedule(const Schedule& c, bool setupOnly)
{
	if (!setupOnly) //setup이 하나가 아닐 때 복사
	{
		// copy code
		_slots = c._slots;
		_classes = c._classes;

		// copy flags of class requirements
		_criteria = c._criteria;

		// copy fitness
		_fitness = c._fitness;
	}
	else //set이 하나일 때는 복사를 해줄 수 없으므로 슬롯이랑 기준만 설정한다.
	{ 

		// reserve space for time-space slots in chromosomes code
		_slots.resize(DAYS_NUM * DAY_HOURS * Configuration::GetInstance().GetNumberOfRooms());

		// reserve space for flags of class requirements
		_criteria.resize(Configuration::GetInstance().GetNumberOfCourseClasses() * Hard_Requirements);
	}

	// copy parameters
	_numberOfCrossoverPoints = c._numberOfCrossoverPoints;
	_mutationSize = c._mutationSize;
	_crossoverProbability = c._crossoverProbability;
	_mutationProbability = c._mutationProbability;
}

// Makes copy ot chromosome
Schedule* Schedule::MakeCopy(bool setupOnly) const
{
	// make object by calling copy constructor and return smart pointer to new object
	return new Schedule(*this, setupOnly);
}

// Makes new chromosome with same setup but with randomly chosen code
Schedule* Schedule::MakeNewFromPrototype() const
{
	// number of time-space slots
	int size = (int)_slots.size();

	// make new chromosome, copy chromosome setup
	Schedule* newChromosome = new Schedule(*this, true);

	// place classes at random position
	const list<CourseClass*>& c = Configuration::GetInstance().GetCourseClasses();
	

	//받아온 c의 모든 원소(수업정보)를 입력해준다.
	for (list<CourseClass*>::const_iterator it = c.begin(); it != c.end(); it++)
	{
		// determine random position of class
		//시간표 list에 해당 class를 넣어 주기 위한 기본 세팅
		//총 방 갯수 설정
		int nr = Configuration::GetInstance().GetNumberOfRooms();
		//수업 시간 설정(만약 수업시간이 3시간 이상일 시 분할 해야함 or 실습이 있는 수업)
		int dur = (*it)->GetDuration();


		//수업 이름 설정 ;
		string course_name = (*it)->GetCourse().GetName();
		//요일 설정
		int day = rand() % DAYS_NUM;
		//룸 설정
		int room = rand() % nr;

		// 실험실이 필요한 수업한 실험실이 있는 교실로 배정
		Room* temp_room = Configuration::GetInstance().GetRoomById(room);

		while (temp_room->IsLab() != (*it)->IsLabRequired())
		{
			room = rand() % nr;
			temp_room = Configuration::GetInstance().GetRoomById(room);		
		}


		//하루 스케줄(9시~20시 30분) 중에 어느 시간대에 배치할 것인지 설정
		

		//점심시간대를 피하기 위한 코드
		int time = rand() % ((DAY_HOURS-5) + 1 - dur);
		if (dur == 4)
		{
			while (time >= 3 && time <= 7)
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
		}
		else
		{
			while (time >=4  && time <= 7)
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
		}

		//특정 과목 시간 고정
		if ((*it)->_fix)
		{
			room = nr - (*it)->room_index;
			day = (*it)->day_index;
			time = ((*it)->time_index - 9)*2;
		}
		else
		{
			//if (day == this->day_index && time >= 4 && time <= 13)
			if (day == (*it)->day_index && time >= 4 && time <= ((*it)->time_index - 9) * 2 *2+dur)
			{
				day = rand() % DAYS_NUM;
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
			}
		}
		


		// list index(list의 마지막 부분을 index로 설정) 설정
		// day * nr * DAY_HOURS은 요일을 결정
		// room * DAY_HOURS은 방을 결정
		// time는 시간대를 결정
		int pos = day * nr * DAY_HOURS + room * DAY_HOURS + time;
		
		


		// fill time-space slots, for each hour of class
		// 새로운 염색체에 위에서 입력한 정보를 기반으로 pos부터 시작해서 입력
		if ((*it)->_ClassCode==NULL)
		{
			for (int i = dur - 1; i >= 0; i--) //거꾸로 입력함
				newChromosome->_slots.at(pos + i).push_back(*it);


			// insert in class table of chromosome
			// CourseClass의 주솟값과 index 위치를 입력
			newChromosome->_classes.insert(pair<CourseClass*, int>(*it, pos));
		}
		else 
		{
			int daySize = DAY_HOURS * nr;
			CourseClass* divided_class = (*it)->_ClassCode;
			int pos1;
			int day1;
			int time1;
			int room1;

			if (newChromosome->Divided_Setting(divided_class, pos1, day1, time1, room1)==1)
			{
				Room* temp_room1 = Configuration::GetInstance().GetRoomById(room1);
				// 실험 조건이 똑같을시 방 맞춰주기
				if (room1 != room && temp_room1->IsLab()== temp_room->IsLab())
				{
					room = room1;
				}

				while (day1 == day || day - day1 == 1 || day1 - day == 1)
				{
					day = rand() % (DAYS_NUM);
				}

				
			}
			
			pos = day * nr * DAY_HOURS + room * DAY_HOURS + time;	
		}

		for (int i = dur - 1; i >= 0; i--) //거꾸로 입력함
		{
			newChromosome->_slots.at(pos + i).push_back(*it);

		}
		newChromosome->_classes.insert(pair<CourseClass*, int>(*it, pos));
	}

	newChromosome->CalculateFitness();

	// return smart pointer
	return newChromosome;
}

// Performes crossover operation using to chromosomes and returns pointer to offspring
// 교차할 부모 염색체를 받아옴
Schedule* Schedule::Crossover(const Schedule& parent2) const
{
	// check probability of crossover operation
	// 교차확률보다 클시 그냥 부모의 염색체를 return함
	
		if (rand() % 100 > _crossoverProbability)
		// no crossover, just copy first parent
		return new Schedule(*this, false);


	// new chromosome object, copy chromosome setup
	// 새로운 염색체를 생성
	Schedule* n = new Schedule(*this, true);
//	return n;
	// number of classes
	int size = (int)_classes.size();

	vector<bool> cp(size);

	// determine crossover point (randomly)
	// 모든 cp[]에 true 값을 입력해줌
	for (int i = _numberOfCrossoverPoints; i > 0; i--)
	{
		
		while (1)
		{
			
			int p = rand() % size;
			//crossover point를 설정해줌
			if (!cp[p])
			{
				cp[p] = true;
				break;
			}
		
		}
	}
	//it1에 원래(부모) 염색체의 값을 받아옴.
	hash_map<CourseClass*, int>::const_iterator it1 = _classes.begin();
	//it2에 받아온(부모) 염색체의 값을 받아옴.
	hash_map<CourseClass*, int>::const_iterator it2 = parent2._classes.begin();

	// make new code by combining parent codes
	bool first = (rand() % 2 == 0);
	// 임의로 먼저 복제할 부모를 선택
	for (int i = 0; i < size; i++)
	{
		if (first)
		{
			// insert class from first parent into new chromosome's calss table
			n->_classes.insert(pair<CourseClass*, int>((*it1).first, (*it1).second));
			// all time-space slots of class are copied
			for (int i = (*it1).first->GetDuration() - 1; i >= 0; i--)
				n->_slots[(*it1).second + i].push_back((*it1).first);
		}
		else
		{
			// insert class from second parent into new chromosome's calss table
			n->_classes.insert(pair<CourseClass*, int>((*it2).first, (*it2).second));
			// all time-space slots of class are copied
			for (int i = (*it2).first->GetDuration() - 1; i >= 0; i--)
				n->_slots[(*it2).second + i].push_back((*it2).first);
		}

		// crossover point
		if (cp[i])
			// change soruce chromosome
 			// 교차지점이 되면 기존에 복사하던 부모를 교체해 준다.
			first = !first;

		// 기존 부모, 받아온 부모 둘 다 한 칸씩 움직여준다.
		it1++;
		it2++;




	}

	n->CalculateFitness();

	// return smart pointer to offspring
	return n;

}


// Performs mutation on chromosome
void Schedule::Mutation()
{
	// check probability of mutation operation
	if (rand() % 100 > _mutationProbability)
		return;

	// number of classes
	int numberOfClasses = (int)_classes.size();
	// number of time-space slots
	int size = (int)_slots.size();
	// move selected number of classes at random position
	for (int i = _mutationSize; i > 0; i--)
	{
		// select random chromosome for movement
		// 어디까지 변이를 줄 것인지 선택
		int mpos = rand() % numberOfClasses;
		int pos1 = 0;
		hash_map<CourseClass*, int>::iterator it = _classes.begin();
		for (; mpos > 0; it++, mpos--) //it 위치를 변이 위치까지 이동시킴
			;

		// current time-space slot used by class
		pos1 = (*it).second; //지금 염색체가 시작되는 index입력

		CourseClass* cc1 = (*it).first; 

		// determine position of class randomly
		int nr = Configuration::GetInstance().GetNumberOfRooms();
		int dur = cc1->GetDuration();
		int day = rand() % DAYS_NUM;
		int room = rand() % nr;
		Room* temp_room = Configuration::GetInstance().GetRoomById(room);
		while (temp_room->IsLab() != cc1->IsLabRequired())
		{
			room = rand() % nr;
			temp_room = Configuration::GetInstance().GetRoomById(room);
		}
	
		int time = rand() % ((DAY_HOURS - 5) + 1 - dur);
		if (dur == 4)
		{
			while (time >= 3 && time <= 7)
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
		}
		else
		{
			while (time >= 4 && time <= 7)
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
		}

		if ((cc1)->_fix)
		{
			room = nr - cc1->room_index;
			day = cc1->day_index;
			time = (cc1->time_index-9)*2;
		}
		else
		{
			if (day == 4 && time >= 4 && time <= (cc1->time_index - 9) * 2 + dur)
			{
				day = rand() % DAYS_NUM;
				time = rand() % ((DAY_HOURS - 5) + 1 - dur);
			}
		}
		
	

		int pos2 = day * nr * DAY_HOURS + room * DAY_HOURS + time;


		if (cc1->_ClassCode != NULL)
		{
			int daySize = DAY_HOURS * nr;
			CourseClass* divided_class = cc1->_ClassCode;
			int pos;
			int day1;
			int time1;
			int room1;

			if (this->Divided_Setting(divided_class, pos, day1, time1, room1) == 1)
			{
				Room* temp_room1 = Configuration::GetInstance().GetRoomById(room1);
				// 실험 조건이 똑같을시 방 맞춰주기
				if (room1 != room && temp_room1->IsLab() == temp_room->IsLab())
				{
					room = room1;
				}

				while (day1 == day || day - day1 == 1 || day1 - day == 1)
				{
						day = rand() % (DAYS_NUM);
				}
			
			}

			pos2 = day * nr * DAY_HOURS + room * DAY_HOURS + time;
		}
	

		// move all time-space slots
		for (int i = dur - 1; i >= 0; i--)
		{
			// remove class hour from current time-space slot
			// 기존 시간표에 있던 정보를 삭제해줌
			list<CourseClass*>& cl = _slots[pos1 + i];
			for (list<CourseClass*>::iterator it = cl.begin(); it != cl.end(); it++)
			{
				if (*it == cc1)
				{
					cl.erase(it);
					break;
				}
			}

			// move class hour to new time-space slot
			// 시간표에 새로운 정보를 입력
			_slots.at(pos2 + i).push_back(cc1);
		}

		// change entry of class table to point to new time-space slots
		_classes[cc1] = pos2;
		
	}

	CalculateFitness();
}

int Schedule::Divided_Setting(CourseClass* course,int& p, int& day, int& time, int& room)
{
	int numberOfRooms = Configuration::GetInstance().GetNumberOfRooms();
	//하루 시간표 길이 설정
	int daySize = DAY_HOURS * numberOfRooms;
	hash_map<CourseClass*, int>::const_iterator it = _classes.begin();
	for (; it != _classes.end(); ++it)
	{
		if ((*it).first == course)
			break;
	}
		

	if (it != _classes.end())
	{
		// coordinate of time-space slot
		 p = (*it).second; //index를 입력 받으므로써 어디에 위치해 있는지 확인
		 day = p / daySize;
		// 해당 index를 daySize(하루 동안의 list 길이)로 나누면 
		// index가 어느 요일인지 확일 할 수 있음
		time = p % daySize;
		//모듈 연산을 통해 나머지를 구하면 그 나머지가 해당 요일의 index가 됨
		//예를 들어 방이 2개인 경우 daySize는 48이 된다.
		//이때 p=49인 경우 49/48 = 1 이므로 화요일임을 알수 있고
		//49%48=1이므로 화요일의 시작 index를 0이라고 생각했을 때
		//p는 화요일 list에 1번 위치에 존재하고 있음을 할 수 있다.

		room = time / DAY_HOURS;
		// 0번 방의 하루 동안의 list의 index를 0~23이고 1번 방 list의 index를 24~47라고 했을 때
		// 그 시간대를 하루 동안의 시간으로 나누면 방 번호를 알 수 있음
		// 위의 예시를 이용하면 (time==1)/24 ==0 이므로 0번 방에 존재함을 알 수 있다.
		time = time % DAY_HOURS;
		return 1;
	}
	else
	{
		return 0;
	}
		
	

}


// Calculates fitness value of chromosome
void Schedule::CalculateFitness()
{
	// chromosome's score
	int score = 0;

	int numberOfRooms = Configuration::GetInstance().GetNumberOfRooms();
	//하루 시간표 길이 설정
	int daySize = DAY_HOURS * numberOfRooms;

	int ci = 0;

	// check criterias and calculate scores for each class in schedule
	for (hash_map<CourseClass*, int>::const_iterator it = _classes.begin(); it != _classes.end(); ++it, ci += Hard_Requirements)
	{
		// coordinate of time-space slot
		int p = (*it).second; //index를 입력 받으므로써 어디에 위치해 있는지 확인
		int day = p / daySize; 
		// 해당 index를 daySize(하루 동안의 list 길이)로 나누면 
		// index가 어느 요일인지 확일 할 수 있음
		int time = p % daySize; 
		//모듈 연산을 통해 나머지를 구하면 그 나머지가 해당 요일의 index가 됨
		//예를 들어 방이 2개인 경우 daySize는 48이 된다.
		//이때 p=49인 경우 49/48 = 1 이므로 화요일임을 알수 있고
		//49%48=1이므로 화요일의 시작 index를 0이라고 생각했을 때
		//p는 화요일 list에 1번 위치에 존재하고 있음을 할 수 있다.

		int room = time / DAY_HOURS; 
		// 0번 방의 하루 동안의 list의 index를 0~23이고 1번 방 list의 index를 24~47라고 했을 때
		// 그 시간대를 하루 동안의 시간으로 나누면 방 번호를 알 수 있음
		// 위의 예시를 이용하면 (time==1)/24 ==0 이므로 0번 방에 존재함을 알 수 있다.
		time = time % DAY_HOURS;
		
	
		// time을 다시 DAY_HOURS로 나머지 연산을 하므로써 각 룸에 맞는 시간대를 정확하게 구할 수 있다.
		// 위의 예시를 이용하면 (time==1)%24 ==1 이므로 1번 시간 대인 
		// 즉 9시30분부터 시작하는 수업임을 알 수 있다.
		int dur = (*it).first->GetDuration();

		// check for room overlapping of classes
		
		bool ro = false;
		for (int i = dur - 1; i >= 0; i--)
		{
			// 하나의 slot에 하나 이상의 CurseClass가 있는 경우
			// vector<list<CourseClass*>> _slots;이므로
			// 하나의 slot에 하나 이하 CurseClass가 존재할 시
			// _slots[].size()<=1이 된다.
			if (_slots[p + i].size() > 1)
			{
				ro = true;
				break;
			}
		}

		// on room overlaping
		// 겹치는 방이 존재하지 않을 경우 score를 하나 증가시킴
		if (!ro)
			score++;
		// 0번 기준인 겹치는 방이 있는지 없는지 여부를 입력
		_criteria[ci + 0] = !ro;

		CourseClass* cc = (*it).first;
		//room 번호를 입력 받고 해당 room의 주솟값을 반환해줌
		Room* r = Configuration::GetInstance().GetRoomById(room);
		// does current room have enough seats
		// 1번 기준인 room에 자리 여부를 입력
		// room에 자리가 충분할 경우 score를 증가 시켜줌
		_criteria[ci + 1] = (r->GetNumberOfSeats() >= cc->GetNumberOfSeats());
		if (_criteria[ci + 1]) 
			score++;

		// does current room have computers if they are required
		// 2번 기준인 실습 여부를 입력
		_criteria[ci + 2] = !cc->IsLabRequired() || (cc->IsLabRequired() && r->IsLab());
		if (_criteria[ci + 2])
			score++;

		bool po = false;// , go = false;
		// check overlapping of classes for professors and student groups
		// 여기서 t는 해당 class를 전체 list에서 봤을 때의 index 값임
		for (int i = numberOfRooms, t = day * daySize + time; i > 0; i--, t += DAY_HOURS)
		{
			// for each hour of class
			for (int i = dur - 1; i >= 0; i--)
				//시간표 검색을 할 때 dur을 기준으로 뒤에서부터 시작함
				//예를 들어 월요일 9시 3시간 수업이면
				//12시부터 검색함
			{
				// check for overlapping with other classes at same time
				const list<CourseClass*>& cl = _slots[t + i]; //뒤에서부터 확인 시작
				for (list<CourseClass*>::const_iterator it = cl.begin(); it != cl.end(); it++)
				{
					if (cc != *it)
					{
						// professor overlaps?
						if (!po && cc->ProfessorOverlaps(**it))
							po = true;

						// student group overlaps?
						//if (!go && cc->GroupsOverlap(**it))
					//		go = true;

						// both type of overlapping? no need to check more
						//if (po && go)
						if (po)
							goto total_overlap;
					}
				}
			}
		}

	total_overlap:

		// professors have no overlaping classes?
		// 3번 기준인 교수 overlaping 여부를 입력
		if (!po)
			score++;
		_criteria[ci + 3] = !po;

		//student  groups 삭제//

		// student groups has no overlaping classes?
		// 4번 기준인 과 overlaping 여부를 입력
		//if (!go)
			score++;
		//_criteria[ci + 4] = !go;
	}

	// calculate fitess value based on score
	
	_fitness = (float)score / (Configuration::GetInstance().GetNumberOfCourseClasses() * DAYS_NUM);
}

// Pointer to global instance of algorithm
Algorithm* Algorithm::_instance = NULL;

// Synchronization of creation and destruction of global instance
CCriticalSection Algorithm::_instanceSect;

// Returns reference to global instance of algorithm
// 전체 알고리즘의 조건 설정
Algorithm& Algorithm::GetInstance()
{
	CSingleLock lock(&_instanceSect, TRUE);

	// global instance doesn't exist?
	if (_instance == NULL)
	{
		// set seed for random generator
		srand(GetTickCount());

		// make prototype of chromosomes
		// Schedule 객체 생성 
		// 교차점 2, 돌연변이 크기 2, 교차확률 80, 돌연변이 확률3
		Schedule* prototype = new Schedule(2, 2, 0, 100);

		// make new global instance of algorithm using chromosome prototype
		// 염색체 수 100, 새롭게 만드는 염색체 수 8, 가장 적합도 좋은 염색체 저장 수 5
		//_instance = new Algorithm(100, 8, 5, prototype, new ScheduleObserver());
		_instance = new Algorithm(100, 100, 5, prototype, new ScheduleObserver());
	//	_instance = new Algorithm(1000, 80, 50, prototype, new ScheduleObserver());
	}

	return *_instance;
}

// Frees memory used by gloval instance
void Algorithm::FreeInstance()
{
	CSingleLock lock(&_instanceSect, TRUE);

	// free memory used by global instance if it exists
	if (_instance != NULL)
	{
		delete _instance->_prototype;
		delete _instance->_observer;
		delete _instance;

		_instance = NULL;
	}
}

// Initializes genetic algorithm
Algorithm::Algorithm(int numberOfChromosomes, int replaceByGeneration, int trackBest,
	Schedule* prototype, ScheduleObserver* observer) : _replaceByGeneration(replaceByGeneration),
	_currentBestSize(0),
	_prototype(prototype),
	_observer(observer),
	_currentGeneration(0),
	_state(AS_USER_STOPED)
{
	// there should be at least 2 chromosomes in population
	// 적어도 2개의 염색체는 존재해야 함
	if (numberOfChromosomes < 2)
		numberOfChromosomes = 2;

	// and algorithm should track at least on of best chromosomes
	// 가장 좋은 염색체는 적어도 하나 존재해야 함
	if (trackBest < 1)
		trackBest = 1;

	//대체 가능 염색체 수는 적어도 한개 이상 존재해야 함
	if (_replaceByGeneration < 1)
		_replaceByGeneration = 1;
	//대체 가능 염색체수는 가장 좋은 염색체를 제외한 염색체 수
	else if (_replaceByGeneration > numberOfChromosomes - trackBest)
		_replaceByGeneration = numberOfChromosomes - trackBest;

	// reserve space for population
	// 염색체 그룹 사이즈 설정
	_chromosomes.resize(numberOfChromosomes);
	
	//Inidicates wheahter chromosome belongs to best chromosome group
	//vector<bool> _bestFlags;
	// _bestFlags는 해당 염색체가 가장 좋은 염색체 그룹에 속해있는지 아닌지 여부를 저장
	_bestFlags.resize(numberOfChromosomes);

	// reserve space for best chromosome group
	// 가장 좋은 염색체 그룹 설정
	_bestChromosomes.resize(trackBest);

	// clear population
	for (int i = (int)_chromosomes.size() - 1; i >= 0; --i)
	{
		_chromosomes[i] = NULL;
		_bestFlags[i] = false;
	}
}

// Frees used resources
Algorithm::~Algorithm()
{
	// clear population by deleting chromosomes 
	for (vector<Schedule*>::iterator it = _chromosomes.begin(); it != _chromosomes.end(); ++it)
	{
		if (*it)
			delete *it;
	}
}

void Algorithm::Start()
{
	//표현형이 없으면 실행 안함
	if (!_prototype)
		return;

	CSingleLock lock(&_stateSect, TRUE);

	// do not run already running algorithm
	// 실행 중이면 시작 안함
	if (_state == AS_RUNNING)
		return;

	// 실행 중으로 설정
	_state = AS_RUNNING;

	lock.Unlock();

	if (_observer)
		// notify observer that execution of algorithm has changed it state
		// 현재 상태를 띄어줌
		_observer->EvolutionStateChanged(_state);

	// clear best chromosome group from previous execution
	// 예전에 있던 best 그룹을 초기화 시켜줌
	ClearBest();

	// initialize new population with chromosomes randomly built using prototype
	int i = 0;
	for (vector<Schedule*>::iterator it = _chromosomes.begin(); it != _chromosomes.end(); ++it, ++i)
	{
		// remove chromosome from previous execution
		// 기존에 있던 염색체를 삭제해주고
		if (*it)
			delete *it;

		// add new chromosome to population
		// 새로운 염색체를 생성
		*it = _prototype->MakeNewFromPrototype();
		// best인지 확인
		AddToBest(i);
	}

	// 현재 세대수 0으로 설정
	_currentGeneration = 0;

	while (1)
	{
		// lock 설정
		lock.Lock();

		// user has stopped execution?
		// 진행 중이 아니라면
		if (_state != AS_RUNNING)
		{
			//lock 해제
			lock.Unlock();
			break;
		}

		// 가장 좋은 염색체를 입력
		Schedule* best = GetBestChromosome();

		// algorithm has reached criteria?
		// 가장 좋은 염색체의 정확도가 100을 넘으면 끝
		if (best->GetFitness() >= 1)
		{
			_state = AS_CRITERIA_STOPPED;
			lock.Unlock();
			break;
		}

		// lock 해제
		lock.Unlock();

		
		// produce offspring
		// 자식 염색체 생성
		vector<Schedule*> offspring;
		// 자식 염색체 갯수 설정
		offspring.resize(_replaceByGeneration);
		for (int j = 0; j < _replaceByGeneration; j++)
		{
			// selects parent randomly
			// 임의로 2개의 염색체 설정
			Schedule* p1 = _chromosomes[rand() % _chromosomes.size()];
			Schedule* p2 = _chromosomes[rand() % _chromosomes.size()];

			//두 염색체를 교차시킴
			offspring[j] =p1->Crossover(*p2);
			//염색체에 변이를 줌
			offspring[j]->Mutation();
		}

		// replace chromosomes of current operation with offspring
		// 자식 염색체와 기존 염색체 교체
		for (int j = 0; j < _replaceByGeneration; j++)
		{
			int ci;
			do
			{
				// select chromosome for replacement randomly
				ci = rand() % (int)_chromosomes.size();

				// protect best chromosomes from replacement
				// best 염색체가 아닌 염색체 index를 선택
			} while (IsInBest(ci));

			// replace chromosomes
			// 염색체 교체
			delete _chromosomes[ci];
			_chromosomes[ci] = offspring[j];

			// try to add new chromosomes in best chromosome group
			// best염색체인지 확인
			AddToBest(ci);
		}

		// algorithm has found new best chromosome
		// best 알고리즘을 검색
		if (best != GetBestChromosome() && _observer)
			// notify observer
			_observer->NewBestChromosome(*GetBestChromosome());

		//세대 수 증가
		_currentGeneration++;
	}

	if (_observer)
		// notify observer that execution of algorithm has changed it state
		_observer->EvolutionStateChanged(_state);
}

// Stops execution of algoruthm
void Algorithm::Stop()
{
	CSingleLock lock(&_stateSect, TRUE);

	// 진행중이라면 상태 바꾸고
	if (_state == AS_RUNNING)
		_state = AS_USER_STOPED;
	//unlock으로 설정
	lock.Unlock();
}

// Returns pointer to best chromosomes in population
// best염색체를 return
Schedule* Algorithm::GetBestChromosome() const
{
	return _chromosomes[_bestChromosomes[0]];
}

// Tries to add chromosomes in best chromosome group
void Algorithm::AddToBest(int chromosomeIndex)
{
	// don't add if new chromosome hasn't fitness big enough for best chromosome group
	// or it is already in the group?
	if ((_currentBestSize == (int)_bestChromosomes.size() &&
		_chromosomes[_bestChromosomes[_currentBestSize - 1]]->GetFitness() >=
		_chromosomes[chromosomeIndex]->GetFitness()) || _bestFlags[chromosomeIndex])
		return;

	// find place for new chromosome
	int i = _currentBestSize;
	for (; i > 0; i--)
	{
		// group is not full?
		if (i < (int)_bestChromosomes.size())
		{
			// position of new chromosomes is found?
			// 기존 best의 정확도가 더 좋으면 break
			if (_chromosomes[_bestChromosomes[i - 1]]->GetFitness() >
				_chromosomes[chromosomeIndex]->GetFitness())
				break;

			// move chromosomes to make room for new
			// 기존 best의 정확도가 더 낮으면 삭제
			_bestChromosomes[i] = _bestChromosomes[i - 1];
		}
		else
			// group is full remove worst chromosomes in the group
			// 
			_bestFlags[_bestChromosomes[i - 1]] = false;
	}

	// store chromosome in best chromosome group
	_bestChromosomes[i] = chromosomeIndex;
	_bestFlags[chromosomeIndex] = true;

	// increase current size if it has not reached the limit yet
	if (_currentBestSize < (int)_bestChromosomes.size())
		_currentBestSize++;
}

// Returns TRUE if chromosome belongs to best chromosome group
bool Algorithm::IsInBest(int chromosomeIndex)
{
	return _bestFlags[chromosomeIndex];
}

// Clears best chromosome group
void Algorithm::ClearBest()
{
	for (int i = (int)_bestFlags.size() - 1; i >= 0; --i)
		_bestFlags[i] = false;

	_currentBestSize = 0;
}
