
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#include <iostream>

#include "../GeneticLibrary/source/Initialization.h"
#include "../GeneticLibrary/source/ChromosomeOperations.h"
#include "../GeneticLibrary/source/MultiValueChromosome.h"
#include "../GeneticLibrary/source/Population.h"
#include "../GeneticLibrary/source/StopCriterias.h"
#include "../GeneticLibrary/source/IncrementalAlgorithm.h"

using namespace std;

using namespace Algorithm;
using namespace Algorithm::StopCriterias;
using namespace Algorithm::SimpleAlgorithms;

class fFitness : public GaFitnessOperation
{
public:

	virtual float GACALL operator ()(const GaChromosome* chromosome) const
	{
		const vector<double>& vals = dynamic_cast<const GaMVArithmeticChromosome<double>*>( chromosome )->GetCode();
		return (float)( 5 * vals[ 0 ] * sin( vals[ 0 ] ) + 1.1 * vals[ 1 ] * sin( vals[ 1 ] ) );
	}

	virtual GaParameters* GACALL MakeParameters() const { return NULL; }

	virtual bool GACALL CheckParameters(const GaParameters& parameters) const { return true; }
};

class fObserver : public GaObserverAdapter
{
	virtual void GACALL NewBestChromosome(const GaChromosome& newChromosome, const GaAlgorithm& algorithm)
	{
		const vector<double>& vals = dynamic_cast<const GaMVArithmeticChromosome<double>&>( newChromosome ).GetCode();
		cout << "New chromosome found:\n";
		cout << "Fitness: " << newChromosome.GetFitness() << endl;
		cout << "x: " << vals[0] << " y: " << vals[1] << endl;
	}

	virtual void GACALL EvolutionStateChanged(GaAlgorithmState newState, const GaAlgorithm& algorithm)
	{
		if( newState == GAS_RUNNING )
			cout << "start\n";
		else if( newState == GAS_CRITERIA_STOPPED )
			cout << "end";
	}
};

int main()
{
	GaInitialize();

	GaChromosomeParams chromosomeParams( 0.03f, 1, true, 0.8f, 1 );

	GaValueIntervalBounds<double> valueInt( 0, 10 );
	GaValueIntervalBounds<double> invValueInt( 0, 10 );
	GaIntervalValueSet<double> valueSet( valueInt, invValueInt, GaGlobalRandomDoubleGenerator, false);

	fFitness fitnessOperation;
	GaChromosomeDomainBlock<double> configBlock( &valueSet,
		GaCrossoverCatalogue::Instance().GetEntryData( "GaMultiValueCrossover" ),
		GaMutationCatalogue::Instance().GetEntryData( "GaFlipMutation" ),
		&fitnessOperation, GaFitnessComparatorCatalogue::Instance().GetEntryData( "GaMinFitnessComparator" ),
		&chromosomeParams );

	GaMVArithmeticChromosome<double> prototype( 2, &configBlock );

	GaPopulationConfiguration populationConfig;
	GaPopulationParameters populationParams( 30, false, true, false, 0, 0 );

	populationConfig.SetParameters( populationParams );
	populationConfig.SetSortComparator( &configBlock.GetFitnessComparator() );

	GaPopulation population( &prototype, &populationConfig );

	GaMultithreadingAlgorithmParams algorithmParams( 1 );
	Algorithm::SimpleAlgorithms::GaIncrementalAlgorithm algorithm( &population, algorithmParams );

	GaGenerationCriteriaParams criteriaParams( 100 );
	algorithm.SetStopCriteria( GaStopCriteriaCatalogue::Instance().GetEntryData( "GaGenerationCriteria" ), &criteriaParams);

	fObserver observer;
	algorithm.SubscribeObserver( &observer );

	algorithm.StartSolving( false );

	algorithm.WaitForThreads();

	GaFinalize();

	return 0;
}
