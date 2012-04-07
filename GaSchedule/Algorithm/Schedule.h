
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#pragma once

#include <list>

#ifdef WIN32

#include <hash_map>
using namespace stdext;

#else

#include <ext/hash_map>
using namespace __gnu_cxx;

#endif

#include "CourseClass.h"

#include "../../GeneticLibrary/source/Initialization.h"

#include "../../GeneticLibrary/source/MultiValueChromosome.h"
#include "../../GeneticLibrary/source/ChromosomeOperations.h"

#include "../../GeneticLibrary/source/Population.h"
#include "../../GeneticLibrary/source/SelectionOperations.h"
#include "../../GeneticLibrary/source/ReplacementOperations.h"

#include "../../GeneticLibrary/source/StopCriterias.h"
#include "../../GeneticLibrary/source/IncrementalAlgorithm.h"

using namespace Threading;
using namespace Chromosome;
using namespace Chromosome::Representation;

using namespace Population;
using namespace Population::ReplacementOperations;
using namespace Population::SelectionOperations;

using namespace Algorithm;
using namespace Algorithm::SimpleAlgorithms;
using namespace Algorithm::StopCriterias;

#if defined(GAL_STL_EXT_MSVC)

	typedef hash_map<CourseClass*, int> CourseClassHashMap;

#else

	class CourseClassHash
	{

	public:

		// Parameters for hash table
		enum
		{
			bucket_size = 4,
			min_buckets = 8
		};	

		// Construct with default comparator
		CourseClassHash() { }

		// Hash function
		size_t GACALL operator()(CourseClass* key) const
		{
			size_t t = (size_t)key;

			ldiv_t rem = ldiv((long)t, 127773);
			rem.rem = 16807 * rem.rem - 2836 * rem.quot;
			if (rem.rem < 0)
				rem.rem += 2147483647;

			return ((size_t)rem.rem);
		}

		// Comparison
		inline bool GACALL operator()(CourseClass* value1, CourseClass* value2) const { return value1 == value2; }

	};

	typedef hash_map<CourseClass*, int, CourseClassHash, CourseClassHash> CourseClassHashMap;

#endif

#define DAY_HOURS	12
#define DAYS_NUM	5

class ScheduleObserver : public GaObserverAdapter
{

private:

	CChildView* _window;

	SysEventObject _event;


	void ReleaseEvent() { SignalEvent( _event ); }

public:

	ScheduleObserver() : _window(NULL) { MakeEvent( _event, 0 ); }

	virtual ~ScheduleObserver() { DeleteEvent( _event ); }

	void WaitEvent() { WaitForEvent( _event ); }
	
	virtual void GACALL NewBestChromosome(const GaChromosome& newChromosome, const GaAlgorithm& algorithm);

	virtual void GACALL EvolutionStateChanged(GaAlgorithmState newState, const GaAlgorithm& algorithm);

	inline void SetWindow(CChildView* window) { _window = window; }

};

class ScheduleCrossover : public GaCrossoverOperation
{

public:

	virtual GaChromosomePtr GACALL operator ()(const GaChromosome* parent1,
									  const GaChromosome* parent2) const;

	virtual GaParameters* GACALL MakeParameters() const { return NULL; }

	virtual bool GACALL CheckParameters(const GaParameters& parameters) const { return true; }

};

class ScheduleMutation : public GaMutationOperation
{

public:

	virtual void GACALL operator ()(GaChromosome* chromosome) const;

	virtual GaParameters* GACALL MakeParameters() const { return NULL; }

	virtual bool GACALL CheckParameters(const GaParameters& parameters) const { return true; }

};

class ScheduleFitness : public GaFitnessOperation
{

public:

	virtual float GACALL operator ()(const GaChromosome* chromosome) const;

	virtual GaParameters* GACALL MakeParameters() const { return NULL; }

	virtual bool GACALL CheckParameters(const GaParameters& parameters) const { return true; }

};

class Schedule : public GaMultiValueChromosome<list<CourseClass*> >
{

	friend class ScheduleCrossover;
	friend class ScheduleMutation;
	friend class ScheduleFitness;
	friend class ScheduleObserver;

private:

	CourseClassHashMap _classes;

	CourseClassHashMap _backupClasses;

	// Flags of class requiroments satisfaction
	mutable vector<bool> _criteria;

public:

	Schedule(GaChromosomeDomainBlock<list<CourseClass*> >* configBlock);

	Schedule(const Schedule& c, bool setupOnly);

	virtual ~Schedule() { }

	virtual GaChromosomePtr GACALL MakeCopy(bool setupOnly) const { return new Schedule( *this, setupOnly ); }

	virtual GaChromosomePtr GACALL MakeNewFromPrototype() const;

	virtual void GACALL PreapareForMutation();

	virtual void GACALL AcceptMutation();

	virtual void GACALL RejectMutation();

	// Returns reference to table of classes
	inline const hash_map<CourseClass*, int>& GetClasses() const { return _classes; }

	// Returns array of flags of class requiroments satisfaction
	inline const vector<bool>& GetCriteria() const { return _criteria; }

	// Return reference to array of time-space slots
	inline const vector<list<CourseClass*> >& GetSlots() const { return _values; }

};

class ScheduleTest
{

private:

	static ScheduleTest _instance;

	GaChromosomeParams* _chromosomeParams;

	ScheduleCrossover _crossoverOperation;

	ScheduleMutation _mutationOperation;

	ScheduleFitness _fitnessOperation;

	GaChromosomeDomainBlock<list<CourseClass*> >* _ccb;

	Schedule* _prototype;

	GaPopulationConfiguration* _populationConfig;

	GaPopulation* _population;

	GaAlgorithm* _algorithm;

	ScheduleObserver _observer;

public:

	inline static ScheduleTest& GetInstance() { return _instance; }

	ScheduleTest();

	~ScheduleTest();

	inline GaAlgorithm* GetAlgorithm() { return _algorithm; }

	inline const GaAlgorithm* GetAlgorithm() const { return _algorithm; }

	inline ScheduleObserver& GetObserver() { return _observer; }

	inline const ScheduleObserver& GetObserver() const { return _observer; }

};
