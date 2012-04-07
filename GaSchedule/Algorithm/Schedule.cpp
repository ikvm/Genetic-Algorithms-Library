
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#include "stdafx.h"
#include <iostream>
#include <vector>

#include "../ChildView.h"
#include "Configuration.h"
#include "Schedule.h"
#include "Room.h"

const char* Days[] = { "MON", "THU", "WEN", "THR", "FRI" };

void ScheduleObserver::NewBestChromosome(const GaChromosome& newChromosome, const GaAlgorithm& algorithm)
{
	if( _window )
		_window->SetSchedule( dynamic_cast<const Schedule*>( &newChromosome ) );
}

void ScheduleObserver::EvolutionStateChanged(GaAlgorithmState newState, const GaAlgorithm& algorithm)
{
	if( _window )
		_window->SetNewState( newState );

	if( newState != GAS_RUNNING )
		ReleaseEvent();
}

GaChromosomePtr ScheduleCrossover::operator ()(const GaChromosome* parent1, const GaChromosome* parent2) const
{
	const Schedule* sch1 = dynamic_cast<const Schedule*>( parent1 );
	const Schedule* sch2 = dynamic_cast<const Schedule*>( parent2 );

	// new chromosome object, copy chromosome setup
	Schedule* n = new Schedule( *sch1, true );

	// number of classes
	int size = (int)sch1->_classes.size();

	// determine crossover point (randomly)
	vector<bool> cp( size );
	for( int i = sch1->GetParameters().GetNumberOfCrossoverPoints(); i > 0; i-- )
	{
		while( 1 )
		{
			int p = GaGlobalRandomIntegerGenerator->Generate( size - 1 );
			if( !cp[ p ] )
			{
				cp[ p ] = true;
				break;
			}
		}
	}

	CourseClassHashMap::const_iterator it1 = sch1->_classes.begin();
	CourseClassHashMap::const_iterator it2 = sch2->_classes.begin();

	// make new code by combining parent codes
	bool first = GaGlobalRandomBoolGenerator->Generate();
	for( int i = 0; i < size; i++ )
	{
		if( first )
		{
			// insert class from first parent into new chromosome's calss table
			n->_classes.insert( pair<CourseClass*, int>( ( *it1 ).first, ( *it1 ).second ) );
			// all time-space slots of class are copied
			for( int i = ( *it1 ).first->GetDuration() - 1; i >= 0; i-- )
				n->_values[ ( *it1 ).second + i ].push_back( ( *it1 ).first );
		}
		else
		{
			// insert class from second parent into new chromosome's calss table
			n->_classes.insert( pair<CourseClass*, int>( ( *it2 ).first, ( *it2 ).second ) );
			// all time-space slots of class are copied
			for( int i = ( *it2 ).first->GetDuration() - 1; i >= 0; i-- )
				n->_values[ ( *it2 ).second + i ].push_back( ( *it2 ).first );
		}

		// crossover point
		if( cp[ i ] )
			// change soruce chromosome
			first = !first;

		it1++;
		it2++;
	}

	// return smart pointer to offspring
	return n;
}

void ScheduleMutation::operator ()(GaChromosome* chromosome) const
{
	Schedule* sch = dynamic_cast<Schedule*>( chromosome );

	// number of classes
	int numberOfClasses = (int)sch->_classes.size();
	// number of time-space slots
	int size = (int)sch->_values.size();

	// move selected number of classes at random position
	for( int i = sch->GetParameters().GetMutationSize(); i > 0; i-- )
	{
		// select ranom chromosome for movement
		int mpos = GaGlobalRandomIntegerGenerator->Generate( numberOfClasses - 1 );
		int pos1 = 0;
		CourseClassHashMap::iterator it = sch->_classes.begin();
		for( ; mpos > 0; it++, mpos-- )
			;
		// current time-space slot used by class
		pos1 = ( *it ).second;

		CourseClass* cc1 = ( *it ).first;

		// determine position of class randomly
		int nr = Configuration::GetInstance().GetNumberOfRooms();
		int dur = cc1->GetDuration();
		int day = GaGlobalRandomIntegerGenerator->Generate( DAYS_NUM  - 1);
		int room = GaGlobalRandomIntegerGenerator->Generate( nr - 1 );
		int time = GaGlobalRandomIntegerGenerator->Generate( DAY_HOURS - dur );
		int pos2 = day * nr * DAY_HOURS + room * DAY_HOURS + time;

		// move all time-space slots
		for( int i = dur - 1; i >= 0; i-- )
		{
			// remove class hour from current time-space slot
			list<CourseClass*>& cl = sch->_values[ pos1 + i ];
			for( list<CourseClass*>::iterator it = cl.begin(); it != cl.end(); it++ )
			{
				if( *it == cc1 )
				{
					cl.erase( it );
					break;
				}
			}

			// move class hour to new time-space slot
			sch->_values.at( pos2 + i ).push_back( cc1 );
		}

		// change entry of class table to point to new time-space slots
		sch->_classes[ cc1 ] = pos2;
	}
}

float ScheduleFitness::operator ()(const GaChromosome* chromosome) const
{
	const Schedule* sch = dynamic_cast<const Schedule*>( chromosome );

	// chromosome's score
	int score = 0;

	int numberOfRooms = Configuration::GetInstance().GetNumberOfRooms();
	int daySize = DAY_HOURS * numberOfRooms;

	int ci = 0;
	// check criterias and calculate scores for each class in schedule
	for( CourseClassHashMap::const_iterator it = sch->_classes.begin(); it != sch->_classes.end(); it++, ci += 5 )
	{
		// coordinate of time-space slot
		int p = ( *it ).second;
		int day = p / daySize;
		int time = p % daySize;
		int room = time / DAY_HOURS;
		time = time % DAY_HOURS;

		int dur = ( *it ).first->GetDuration();

		// check for room overlapping of classes
		bool ro = false;
		for( int i = dur - 1; i >= 0; i-- )
		{
			if( sch->_values[ p + i ].size() > 1 )
			{
				ro = true;
				break;
			}
		}

		// on room overlaping
		if( !ro )
			score++;

		sch->_criteria[ ci + 0 ] = !ro;

		CourseClass* cc = ( *it ).first;
		Room* r = Configuration::GetInstance().GetRoomById( room );

		// does current room have enough seats
		sch->_criteria[ ci + 1 ] = r->GetNumberOfSeats() >= cc->GetNumberOfSeats();
		if( sch->_criteria[ ci + 1 ] )
			score++;

		// does current room have computers if they are required
		sch->_criteria[ ci + 2 ] = !cc->IsLabRequired() || ( cc->IsLabRequired() && r->IsLab() );
		if( sch->_criteria[ ci + 2 ] )
			score++;

		bool po = false, go = false;
		// check overlapping of classes for professors and student groups
		for( int i = numberOfRooms, t = day * daySize + time; i > 0; i--, t += DAY_HOURS )
		{
			for( int i = dur - 1; i >= 0; i-- )
			{
				const list<CourseClass*>& cl = sch->_values[ t + i ];

				for( list<CourseClass*>::const_iterator it = cl.begin(); it != cl.end(); it++ )
				{
					if( cc != *it )
					{
						// professor overlaps?
						if( !po && cc->ProfessorOverlaps( **it ) )
							po = true;

						// student group overlaps?
						if( !go && cc->GroupsOverlap( **it ) )
							go = true;

						// both type of overlapping? no need to check more
						if( po && go )
							goto total_overlap;
					}
				}
			}
		}

total_overlap:

		// professors have no overlaping classes?
		if( !po )
			score++;
		sch->_criteria[ ci + 3 ] = !po;

		// student groups has no overlaping classes?
		if( !go )
			score++;
		sch->_criteria[ ci + 4 ] = !go;
	}

	// calculate fitess value based on score
	return (float)score / ( Configuration::GetInstance().GetNumberOfCourseClasses() * DAYS_NUM );
}

Schedule::Schedule(GaChromosomeDomainBlock<list<CourseClass*> >* configBlock) : 
				   GaMultiValueChromosome<list<CourseClass*> >(configBlock)
{
	// reserve space for time-space slots in chromosomes code
	_values.resize( DAYS_NUM * DAY_HOURS * Configuration::GetInstance().GetNumberOfRooms() );

	// reserve space for flags of class requirements
	_criteria.resize( Configuration::GetInstance().GetNumberOfCourseClasses() * 5 );
}

Schedule::Schedule(const Schedule& c, bool setupOnly) :
				   GaMultiValueChromosome<list<CourseClass*> >(c, setupOnly)
{
	if( !setupOnly )
	{
		// copy 
		_classes = c._classes;

		// copy flags of class requirements
		_criteria = c._criteria;
	}
	else
	{
		// reserve space for time-space slots in chromosomes code
		_values.resize( DAYS_NUM * DAY_HOURS * Configuration::GetInstance().GetNumberOfRooms() );

		// reserve space for flags of class requirements
		_criteria.resize( Configuration::GetInstance().GetNumberOfCourseClasses() * 5 );
	}
}

GaChromosomePtr Schedule::MakeNewFromPrototype() const
{
	// number of time-space slots
	int size = (int)_values.size();

	// make new chromosome, copy chromosome setup
	Schedule* newChromosome = new Schedule( *this, true );

	// place classes at random position
	const list<CourseClass*>& c = Configuration::GetInstance().GetCourseClasses();
	for( list<CourseClass*>::const_iterator it = c.begin(); it != c.end(); it++ )
	{
		// determine random position of class
		int nr = Configuration::GetInstance().GetNumberOfRooms();
		int dur = ( *it )->GetDuration();
		int day = GaGlobalRandomIntegerGenerator->Generate( DAYS_NUM - 1 );
		int room = GaGlobalRandomIntegerGenerator->Generate( nr - 1 );
		int time = GaGlobalRandomIntegerGenerator->Generate( DAY_HOURS - dur );
		int pos = day * nr * DAY_HOURS + room * DAY_HOURS + time;

		// fill time-space slots, for each hour of class
		for( int i = dur - 1; i >= 0; i-- )
			newChromosome->_values.at( pos + i ).push_back( *it );

		// insert in class table of chromosome
		newChromosome->_classes.insert( pair<CourseClass*, int>( *it, pos ) );
	}

	// return smart pointer
	return newChromosome;
}

void GACALL Schedule::PreapareForMutation()
{
	_backupClasses = _classes;

	GaMultiValueChromosome<list<CourseClass*> >::PreapareForMutation();
}

void GACALL Schedule::AcceptMutation()
{
	_backupClasses.clear();

	GaMultiValueChromosome<list<CourseClass*> >::AcceptMutation();
}

void GACALL Schedule::RejectMutation()
{
	_classes = _backupClasses;
	_backupClasses.clear();

	GaMultiValueChromosome<list<CourseClass*> >::RejectMutation();
}

ScheduleTest ScheduleTest::_instance;

ScheduleTest::ScheduleTest()
{
	// initialize GAL internal structures
	GaInitialize();
	
	// make chromosome parameters
	// crossover probability: 80%
	// crossover points: 2
	// no "improving only mutations"
	// mutation probability: 3%
	// number of moved classes per mutation: 2
	_chromosomeParams = new GaChromosomeParams( 0.03F, 2, false, 0.8F, 2 );

	// make CCB with fallowing setup:
	// there are no value set
	// with ScheduleCrossover, ScheduleMutation, ScheduleFitness genetic operations
	// set fittnes comparator for maximizing fitness value
	// use previously defined chromosome's parameters
	_ccb = new GaChromosomeDomainBlock<list<CourseClass*> >( NULL, 0, &_crossoverOperation, &_mutationOperation, &_fitnessOperation,
		GaFitnessComparatorCatalogue::Instance().GetEntryData( "GaMaxFitnessComparator" ), _chromosomeParams );

	// make prototype of chromosome
	_prototype = new Schedule( _ccb );

	// make population parameters
	// number of chromosomes in population: 100
	// population always has fixed number of chromosomes
	// population is not sorted
	// non-transformed(non-scaled) fitness values are used for sorting and tracking chromosomes
	// population tracks 5 best and 5 worst chromosomes
	GaPopulationParameters populationParams( 100, false, false, false, 5, 5 );

	// make parameters for selection operation
	// selection will choose 16 chromosomes
	// but only 8 best of them will be stored in selection result set
	// there will be no duplicates of chromosomes in result set
	GaSelectRandomBestParams selParam( 8, false, 16 );

	// make parameters for replacement operation
	// replace 8 chromosomes
	// but keep 2 best chromosomes in population
	GaReplaceElitismParams repParam( 8, 2 );

	// make parameters for coupling operation
	// coupling operation will produce 8 new chromosomes from selected parents
	GaCouplingParams coupParam( 8, false );

	// make population configuration
	// use defined population parameters
	// use same comparator for sorting as comparator used by chromosomes
	// use selection operation which randomly selects chromosomes
	// use replacement operation which randomly chooses chromosomes from population
	// which are going to be replaced, but keeps best chromosomes
	// use simple coupling
	// disable scaling
	_populationConfig = new GaPopulationConfiguration ( populationParams, &_ccb->GetFitnessComparator(),
		GaSelectionCatalogue::Instance().GetEntryData( "GaSelectRandom" ), &selParam,
		GaReplacementCatalogue::Instance().GetEntryData( "GaReplaceRandom" ), &repParam,
		GaCouplingCatalogue::Instance().GetEntryData( "GaSimpleCoupling" ), &coupParam,
		NULL, NULL );

	// make population
	// with previously defined prototype of chromosomes and population configuration
	_population = new GaPopulation( _prototype, _populationConfig );

	// make parameters for genetic algorithms
	// algorithm will use two workers
	GaMultithreadingAlgorithmParams algorithmParams( 2 );
	// make incremental algorithm with periously defined population and parameters
	_algorithm = new GaIncrementalAlgorithm( _population, algorithmParams );

	// make parameters for stop criteria based on fitness value
	// stop when best chromosome reaches fitness value of 1
	GaFitnessCriteriaParams criteriaParams( 1, GFC_EQUALS_TO, GSV_BEST_FITNESS );

	// sets algorithm's stop criteria (base on fitness value) and its parameters
	_algorithm->SetStopCriteria( GaStopCriteriaCatalogue::Instance().GetEntryData( "GaFitnessCriteria" ), 
		&criteriaParams );

	// subscribe observer
	_algorithm->SubscribeObserver( &_observer );
}

ScheduleTest::~ScheduleTest()
{
	delete _algorithm;

	delete _population;
	delete _populationConfig;

	delete _prototype;
	delete _ccb;
	delete _chromosomeParams;

	// Free resources used by GAL
	GaFinalize();
}
