
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
	//30�� ������ ���� // �������� ����(�� string�� ����)�� ��������
	//�ð�ǥ�� �ϳ��� string���� ����
	
	_slots.resize(DAYS_NUM * DAY_HOURS * Configuration::GetInstance().GetNumberOfRooms());
	


	// reserve space for flags of class requirements
	// �� CourseClasses�� Hard_Reqirements�� ���������ν� �������Ѿ��� �� ������ �������ش�.
	// ���� ��� CourseClass�� 3���� ��� hardRequirements�� ���ϹǷν� criteria�� ���̸� ������ �� �ִ�.
	_criteria.resize(Configuration::GetInstance().GetNumberOfCourseClasses() * Hard_Requirements);
}

// Copy constructor
Schedule::Schedule(const Schedule& c, bool setupOnly)
{
	if (!setupOnly) //setup�� �ϳ��� �ƴ� �� ����
	{
		// copy code
		_slots = c._slots;
		_classes = c._classes;

		// copy flags of class requirements
		_criteria = c._criteria;

		// copy fitness
		_fitness = c._fitness;
	}
	else //set�� �ϳ��� ���� ���縦 ���� �� �����Ƿ� �����̶� ���ظ� �����Ѵ�.
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
	

	//�޾ƿ� c�� ��� ����(��������)�� �Է����ش�.
	for (list<CourseClass*>::const_iterator it = c.begin(); it != c.end(); it++)
	{
		// determine random position of class
		//�ð�ǥ list�� �ش� class�� �־� �ֱ� ���� �⺻ ����
		//�� �� ���� ����
		int nr = Configuration::GetInstance().GetNumberOfRooms();
		//���� �ð� ����(���� �����ð��� 3�ð� �̻��� �� ���� �ؾ��� or �ǽ��� �ִ� ����)
		int dur = (*it)->GetDuration();


		//���� �̸� ���� ;
		string course_name = (*it)->GetCourse().GetName();
		//���� ����
		int day = rand() % DAYS_NUM;
		//�� ����
		int room = rand() % nr;

		// ������� �ʿ��� ������ ������� �ִ� ���Ƿ� ����
		Room* temp_room = Configuration::GetInstance().GetRoomById(room);

		while (temp_room->IsLab() != (*it)->IsLabRequired())
		{
			room = rand() % nr;
			temp_room = Configuration::GetInstance().GetRoomById(room);		
		}


		//�Ϸ� ������(9��~20�� 30��) �߿� ��� �ð��뿡 ��ġ�� ������ ����
		

		//���ɽð��븦 ���ϱ� ���� �ڵ�
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

		//Ư�� ���� �ð� ����
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
		


		// list index(list�� ������ �κ��� index�� ����) ����
		// day * nr * DAY_HOURS�� ������ ����
		// room * DAY_HOURS�� ���� ����
		// time�� �ð��븦 ����
		int pos = day * nr * DAY_HOURS + room * DAY_HOURS + time;
		
		


		// fill time-space slots, for each hour of class
		// ���ο� ����ü�� ������ �Է��� ������ ������� pos���� �����ؼ� �Է�
		if ((*it)->_ClassCode==NULL)
		{
			for (int i = dur - 1; i >= 0; i--) //�Ųٷ� �Է���
				newChromosome->_slots.at(pos + i).push_back(*it);


			// insert in class table of chromosome
			// CourseClass�� �ּڰ��� index ��ġ�� �Է�
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
				// ���� ������ �Ȱ����� �� �����ֱ�
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

		for (int i = dur - 1; i >= 0; i--) //�Ųٷ� �Է���
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
// ������ �θ� ����ü�� �޾ƿ�
Schedule* Schedule::Crossover(const Schedule& parent2) const
{
	// check probability of crossover operation
	// ����Ȯ������ Ŭ�� �׳� �θ��� ����ü�� return��
	
		if (rand() % 100 > _crossoverProbability)
		// no crossover, just copy first parent
		return new Schedule(*this, false);


	// new chromosome object, copy chromosome setup
	// ���ο� ����ü�� ����
	Schedule* n = new Schedule(*this, true);
//	return n;
	// number of classes
	int size = (int)_classes.size();

	vector<bool> cp(size);

	// determine crossover point (randomly)
	// ��� cp[]�� true ���� �Է�����
	for (int i = _numberOfCrossoverPoints; i > 0; i--)
	{
		
		while (1)
		{
			
			int p = rand() % size;
			//crossover point�� ��������
			if (!cp[p])
			{
				cp[p] = true;
				break;
			}
		
		}
	}
	//it1�� ����(�θ�) ����ü�� ���� �޾ƿ�.
	hash_map<CourseClass*, int>::const_iterator it1 = _classes.begin();
	//it2�� �޾ƿ�(�θ�) ����ü�� ���� �޾ƿ�.
	hash_map<CourseClass*, int>::const_iterator it2 = parent2._classes.begin();

	// make new code by combining parent codes
	bool first = (rand() % 2 == 0);
	// ���Ƿ� ���� ������ �θ� ����
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
 			// ���������� �Ǹ� ������ �����ϴ� �θ� ��ü�� �ش�.
			first = !first;

		// ���� �θ�, �޾ƿ� �θ� �� �� �� ĭ�� �������ش�.
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
		// ������ ���̸� �� ������ ����
		int mpos = rand() % numberOfClasses;
		int pos1 = 0;
		hash_map<CourseClass*, int>::iterator it = _classes.begin();
		for (; mpos > 0; it++, mpos--) //it ��ġ�� ���� ��ġ���� �̵���Ŵ
			;

		// current time-space slot used by class
		pos1 = (*it).second; //���� ����ü�� ���۵Ǵ� index�Է�

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
				// ���� ������ �Ȱ����� �� �����ֱ�
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
			// ���� �ð�ǥ�� �ִ� ������ ��������
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
			// �ð�ǥ�� ���ο� ������ �Է�
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
	//�Ϸ� �ð�ǥ ���� ����
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
		 p = (*it).second; //index�� �Է� �����Ƿν� ��� ��ġ�� �ִ��� Ȯ��
		 day = p / daySize;
		// �ش� index�� daySize(�Ϸ� ������ list ����)�� ������ 
		// index�� ��� �������� Ȯ�� �� �� ����
		time = p % daySize;
		//��� ������ ���� �������� ���ϸ� �� �������� �ش� ������ index�� ��
		//���� ��� ���� 2���� ��� daySize�� 48�� �ȴ�.
		//�̶� p=49�� ��� 49/48 = 1 �̹Ƿ� ȭ�������� �˼� �ְ�
		//49%48=1�̹Ƿ� ȭ������ ���� index�� 0�̶�� �������� ��
		//p�� ȭ���� list�� 1�� ��ġ�� �����ϰ� ������ �� �� �ִ�.

		room = time / DAY_HOURS;
		// 0�� ���� �Ϸ� ������ list�� index�� 0~23�̰� 1�� �� list�� index�� 24~47��� ���� ��
		// �� �ð��븦 �Ϸ� ������ �ð����� ������ �� ��ȣ�� �� �� ����
		// ���� ���ø� �̿��ϸ� (time==1)/24 ==0 �̹Ƿ� 0�� �濡 �������� �� �� �ִ�.
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
	//�Ϸ� �ð�ǥ ���� ����
	int daySize = DAY_HOURS * numberOfRooms;

	int ci = 0;

	// check criterias and calculate scores for each class in schedule
	for (hash_map<CourseClass*, int>::const_iterator it = _classes.begin(); it != _classes.end(); ++it, ci += Hard_Requirements)
	{
		// coordinate of time-space slot
		int p = (*it).second; //index�� �Է� �����Ƿν� ��� ��ġ�� �ִ��� Ȯ��
		int day = p / daySize; 
		// �ش� index�� daySize(�Ϸ� ������ list ����)�� ������ 
		// index�� ��� �������� Ȯ�� �� �� ����
		int time = p % daySize; 
		//��� ������ ���� �������� ���ϸ� �� �������� �ش� ������ index�� ��
		//���� ��� ���� 2���� ��� daySize�� 48�� �ȴ�.
		//�̶� p=49�� ��� 49/48 = 1 �̹Ƿ� ȭ�������� �˼� �ְ�
		//49%48=1�̹Ƿ� ȭ������ ���� index�� 0�̶�� �������� ��
		//p�� ȭ���� list�� 1�� ��ġ�� �����ϰ� ������ �� �� �ִ�.

		int room = time / DAY_HOURS; 
		// 0�� ���� �Ϸ� ������ list�� index�� 0~23�̰� 1�� �� list�� index�� 24~47��� ���� ��
		// �� �ð��븦 �Ϸ� ������ �ð����� ������ �� ��ȣ�� �� �� ����
		// ���� ���ø� �̿��ϸ� (time==1)/24 ==0 �̹Ƿ� 0�� �濡 �������� �� �� �ִ�.
		time = time % DAY_HOURS;
		
	
		// time�� �ٽ� DAY_HOURS�� ������ ������ �ϹǷν� �� �뿡 �´� �ð��븦 ��Ȯ�ϰ� ���� �� �ִ�.
		// ���� ���ø� �̿��ϸ� (time==1)%24 ==1 �̹Ƿ� 1�� �ð� ���� 
		// �� 9��30�к��� �����ϴ� �������� �� �� �ִ�.
		int dur = (*it).first->GetDuration();

		// check for room overlapping of classes
		
		bool ro = false;
		for (int i = dur - 1; i >= 0; i--)
		{
			// �ϳ��� slot�� �ϳ� �̻��� CurseClass�� �ִ� ���
			// vector<list<CourseClass*>> _slots;�̹Ƿ�
			// �ϳ��� slot�� �ϳ� ���� CurseClass�� ������ ��
			// _slots[].size()<=1�� �ȴ�.
			if (_slots[p + i].size() > 1)
			{
				ro = true;
				break;
			}
		}

		// on room overlaping
		// ��ġ�� ���� �������� ���� ��� score�� �ϳ� ������Ŵ
		if (!ro)
			score++;
		// 0�� ������ ��ġ�� ���� �ִ��� ������ ���θ� �Է�
		_criteria[ci + 0] = !ro;

		CourseClass* cc = (*it).first;
		//room ��ȣ�� �Է� �ް� �ش� room�� �ּڰ��� ��ȯ����
		Room* r = Configuration::GetInstance().GetRoomById(room);
		// does current room have enough seats
		// 1�� ������ room�� �ڸ� ���θ� �Է�
		// room�� �ڸ��� ����� ��� score�� ���� ������
		_criteria[ci + 1] = (r->GetNumberOfSeats() >= cc->GetNumberOfSeats());
		if (_criteria[ci + 1]) 
			score++;

		// does current room have computers if they are required
		// 2�� ������ �ǽ� ���θ� �Է�
		_criteria[ci + 2] = !cc->IsLabRequired() || (cc->IsLabRequired() && r->IsLab());
		if (_criteria[ci + 2])
			score++;

		bool po = false;// , go = false;
		// check overlapping of classes for professors and student groups
		// ���⼭ t�� �ش� class�� ��ü list���� ���� ���� index ����
		for (int i = numberOfRooms, t = day * daySize + time; i > 0; i--, t += DAY_HOURS)
		{
			// for each hour of class
			for (int i = dur - 1; i >= 0; i--)
				//�ð�ǥ �˻��� �� �� dur�� �������� �ڿ������� ������
				//���� ��� ������ 9�� 3�ð� �����̸�
				//12�ú��� �˻���
			{
				// check for overlapping with other classes at same time
				const list<CourseClass*>& cl = _slots[t + i]; //�ڿ������� Ȯ�� ����
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
		// 3�� ������ ���� overlaping ���θ� �Է�
		if (!po)
			score++;
		_criteria[ci + 3] = !po;

		//student  groups ����//

		// student groups has no overlaping classes?
		// 4�� ������ �� overlaping ���θ� �Է�
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
// ��ü �˰����� ���� ����
Algorithm& Algorithm::GetInstance()
{
	CSingleLock lock(&_instanceSect, TRUE);

	// global instance doesn't exist?
	if (_instance == NULL)
	{
		// set seed for random generator
		srand(GetTickCount());

		// make prototype of chromosomes
		// Schedule ��ü ���� 
		// ������ 2, �������� ũ�� 2, ����Ȯ�� 80, �������� Ȯ��3
		Schedule* prototype = new Schedule(2, 2, 0, 100);

		// make new global instance of algorithm using chromosome prototype
		// ����ü �� 100, ���Ӱ� ����� ����ü �� 8, ���� ���յ� ���� ����ü ���� �� 5
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
	// ��� 2���� ����ü�� �����ؾ� ��
	if (numberOfChromosomes < 2)
		numberOfChromosomes = 2;

	// and algorithm should track at least on of best chromosomes
	// ���� ���� ����ü�� ��� �ϳ� �����ؾ� ��
	if (trackBest < 1)
		trackBest = 1;

	//��ü ���� ����ü ���� ��� �Ѱ� �̻� �����ؾ� ��
	if (_replaceByGeneration < 1)
		_replaceByGeneration = 1;
	//��ü ���� ����ü���� ���� ���� ����ü�� ������ ����ü ��
	else if (_replaceByGeneration > numberOfChromosomes - trackBest)
		_replaceByGeneration = numberOfChromosomes - trackBest;

	// reserve space for population
	// ����ü �׷� ������ ����
	_chromosomes.resize(numberOfChromosomes);
	
	//Inidicates wheahter chromosome belongs to best chromosome group
	//vector<bool> _bestFlags;
	// _bestFlags�� �ش� ����ü�� ���� ���� ����ü �׷쿡 �����ִ��� �ƴ��� ���θ� ����
	_bestFlags.resize(numberOfChromosomes);

	// reserve space for best chromosome group
	// ���� ���� ����ü �׷� ����
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
	//ǥ������ ������ ���� ����
	if (!_prototype)
		return;

	CSingleLock lock(&_stateSect, TRUE);

	// do not run already running algorithm
	// ���� ���̸� ���� ����
	if (_state == AS_RUNNING)
		return;

	// ���� ������ ����
	_state = AS_RUNNING;

	lock.Unlock();

	if (_observer)
		// notify observer that execution of algorithm has changed it state
		// ���� ���¸� �����
		_observer->EvolutionStateChanged(_state);

	// clear best chromosome group from previous execution
	// ������ �ִ� best �׷��� �ʱ�ȭ ������
	ClearBest();

	// initialize new population with chromosomes randomly built using prototype
	int i = 0;
	for (vector<Schedule*>::iterator it = _chromosomes.begin(); it != _chromosomes.end(); ++it, ++i)
	{
		// remove chromosome from previous execution
		// ������ �ִ� ����ü�� �������ְ�
		if (*it)
			delete *it;

		// add new chromosome to population
		// ���ο� ����ü�� ����
		*it = _prototype->MakeNewFromPrototype();
		// best���� Ȯ��
		AddToBest(i);
	}

	// ���� ����� 0���� ����
	_currentGeneration = 0;

	while (1)
	{
		// lock ����
		lock.Lock();

		// user has stopped execution?
		// ���� ���� �ƴ϶��
		if (_state != AS_RUNNING)
		{
			//lock ����
			lock.Unlock();
			break;
		}

		// ���� ���� ����ü�� �Է�
		Schedule* best = GetBestChromosome();

		// algorithm has reached criteria?
		// ���� ���� ����ü�� ��Ȯ���� 100�� ������ ��
		if (best->GetFitness() >= 1)
		{
			_state = AS_CRITERIA_STOPPED;
			lock.Unlock();
			break;
		}

		// lock ����
		lock.Unlock();

		
		// produce offspring
		// �ڽ� ����ü ����
		vector<Schedule*> offspring;
		// �ڽ� ����ü ���� ����
		offspring.resize(_replaceByGeneration);
		for (int j = 0; j < _replaceByGeneration; j++)
		{
			// selects parent randomly
			// ���Ƿ� 2���� ����ü ����
			Schedule* p1 = _chromosomes[rand() % _chromosomes.size()];
			Schedule* p2 = _chromosomes[rand() % _chromosomes.size()];

			//�� ����ü�� ������Ŵ
			offspring[j] =p1->Crossover(*p2);
			//����ü�� ���̸� ��
			offspring[j]->Mutation();
		}

		// replace chromosomes of current operation with offspring
		// �ڽ� ����ü�� ���� ����ü ��ü
		for (int j = 0; j < _replaceByGeneration; j++)
		{
			int ci;
			do
			{
				// select chromosome for replacement randomly
				ci = rand() % (int)_chromosomes.size();

				// protect best chromosomes from replacement
				// best ����ü�� �ƴ� ����ü index�� ����
			} while (IsInBest(ci));

			// replace chromosomes
			// ����ü ��ü
			delete _chromosomes[ci];
			_chromosomes[ci] = offspring[j];

			// try to add new chromosomes in best chromosome group
			// best����ü���� Ȯ��
			AddToBest(ci);
		}

		// algorithm has found new best chromosome
		// best �˰����� �˻�
		if (best != GetBestChromosome() && _observer)
			// notify observer
			_observer->NewBestChromosome(*GetBestChromosome());

		//���� �� ����
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

	// �������̶�� ���� �ٲٰ�
	if (_state == AS_RUNNING)
		_state = AS_USER_STOPED;
	//unlock���� ����
	lock.Unlock();
}

// Returns pointer to best chromosomes in population
// best����ü�� return
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
			// ���� best�� ��Ȯ���� �� ������ break
			if (_chromosomes[_bestChromosomes[i - 1]]->GetFitness() >
				_chromosomes[chromosomeIndex]->GetFitness())
				break;

			// move chromosomes to make room for new
			// ���� best�� ��Ȯ���� �� ������ ����
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
