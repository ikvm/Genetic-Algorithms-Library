
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#include "StdAfx.h"
#include "TspAlgorithm.h"

#include "../GeneticLibrary/source/SelectionOperations.h"
#include "../GeneticLibrary/source/ReplacementOperations.h"

using namespace SelectionOperations;
using namespace ReplacementOperations;

GaChromosomePtr TspChromosome::MakeNewFromPrototype() const
{
	TspChromosome* g = new TspChromosome( (GaChromosomeDomainBlock<const TspCity*>*)_configBlock );

	TspCities::GetInstance().GetCities( g->_values );

	int size = (int)g->_values.size();
	for( int i = size - 1; i >= 0; i-- )
	{
		int p = GaGlobalRandomIntegerGenerator->Generate( size - 1 );

		const TspCity* t = g->_values[ i ];
		g->_values[ i ] = g->_values[ p ];
		g->_values[ p ] = t;
	}

	return g;
}

int TspChromosome::GetCityPosition(const TspCity* city) const
{
	for( int i = (int)_values.size() - 1; i >= 0 ; i-- )
	{
		if( _values[ i ] == city )
			return i;
	}

	return -1;
}

GaChromosomePtr TspCrossover::operator ()(const GaChromosome* parent1,
										  const GaChromosome* parent2) const
{
	const TspChromosome* p1 = dynamic_cast<const TspChromosome*>( parent1 );
	const TspChromosome* p2 = dynamic_cast<const TspChromosome*>( parent2 );

	int numberOfCities = TspCities::GetInstance().GetCount();

	GaChromosomePtr newChromosome = p1->MakeCopy( true );

	TspChromosome* child = dynamic_cast<TspChromosome*>( &(*newChromosome) );

	bool* taken_form_first = new bool[ numberOfCities ];
	bool* taken_from_second = new bool[ numberOfCities ];

	for( int i = numberOfCities - 1; i >= 0; i-- )
		taken_form_first[ i ] = taken_from_second[ i ] = false;

	const TspCity* currentCity = p1->GetAt( GaGlobalRandomIntegerGenerator->Generate( numberOfCities - 1 ) );

	while( 1 )
	{
		int first_parent_city_position = p1->GetCityPosition( currentCity );
		int second_parent_city_position = p2->GetCityPosition( currentCity );

		child->Insert( child->GetCodeSize(), &GaChromosomeValue<const TspCity*>( currentCity ), 1 );

		taken_form_first[ first_parent_city_position ] = true;
		taken_from_second[ second_parent_city_position ] = true;

		const TspCity* nextCity = NULL;

		for( int j = 0; j < 2; j++ )
		{
			int pos = j ? first_parent_city_position : second_parent_city_position;
			bool* taken = j ? taken_form_first : taken_from_second;
			const TspChromosome* c = j ? p1 : p2;

			if( pos )
			{
				if( !taken[ pos - 1 ] )
					SelectNextCity( currentCity, &nextCity, c->GetAt( pos - 1 ) );

				if( pos < numberOfCities - 1 )
				{
					if( !taken[ pos + 1 ] )
						SelectNextCity( currentCity, &nextCity, c->GetAt( pos + 1 ) );
				}
				else
				{
					if( !taken[ 0 ] )
						SelectNextCity( currentCity, &nextCity, c->GetAt( 0 ) );
				}
			}
			else
			{
				if( !taken[ numberOfCities - 1 ] )
					SelectNextCity( currentCity, &nextCity, c->GetAt( numberOfCities - 1 ) );

				if( !taken[ pos + 1 ] )
					SelectNextCity( currentCity, &nextCity, c->GetAt( pos + 1 ) );
			}
		}

		if( !nextCity )
		{
			int startRandom = GaGlobalRandomIntegerGenerator->Generate( numberOfCities - 1 );

			for( int j = numberOfCities - 1; j >= 0 ; j-- )
			{
				int p = ( startRandom + j ) % numberOfCities;
				if( !taken_form_first[ p ] )
				{
					nextCity = p1->GetAt( p );
					break;
				}
			}
		}

		if( !nextCity )
			break;

		currentCity = nextCity;
	}

	delete taken_form_first;
	delete taken_from_second;

	return newChromosome;
}

float TspFitness::operator ()(const GaChromosome* chromosome) const
{
	const TspChromosome* c = dynamic_cast<const TspChromosome*>( chromosome );
	const vector<const TspCity*>& v = c->GetCode();

	float fitness = 0;
	for( int i = (int)v.size() - 1; i >= 0 ; i-- )
		fitness += v[ i ]->GetDistance( *v[ i ? i - 1 : v.size() - 1 ] );

	return fitness;
}

TSP TSP::_instance;

TSP::TSP()
{
	GaInitialize();

	_ccb = new GaChromosomeDomainBlock<const TspCity*>( NULL, 0, new TspCrossover(), GaMutationCatalogue::Instance().GetEntryData( "GaSwapMutation" ),
		new TspFitness(), GaFitnessComparatorCatalogue::Instance().GetEntryData( "GaMinFitnessComparator" ),
		new GaChromosomeParams( 0.3f, 2, false, 0.8f, 2 ) );

	_prototype = new TspChromosome( _ccb );

	GaPopulationParameters populationParams( 100, false, true, false, 0, 0 );
	GaSelectRandomBestParams selectParams( 8, true, 10 );
	GaReplaceElitismParams replaceParams( 8, 10 );
	GaCouplingParams couplingParamss( 8, true );

	_populationConfiguration = new GaPopulationConfiguration( populationParams, &_ccb->GetFitnessComparator(),
		GaSelectionCatalogue::Instance().GetEntryData( "GaSelectRandomBest" ), &selectParams,
		GaReplacementCatalogue::Instance().GetEntryData( "GaReplaceRandom" ), &replaceParams,
		GaCouplingCatalogue::Instance().GetEntryData( "GaSimpleCoupling" ), &couplingParamss,
		NULL, NULL);

	_population = new GaPopulation( _prototype, _populationConfiguration );

	GaMultithreadingAlgorithmParams algParam( 1 );
	_algorithm = new GaIncrementalAlgorithm( _population, algParam );

	GaStopCriteria* criteria = GaStopCriteriaCatalogue::Instance().GetEntryData( "GaFitnessProgressCriteria" );
	GaFitnessProgressCriteriaParams critParam( 1, true, GFC_LESS_THEN, GSV_BEST_FITNESS, 50000 );
	_algorithm->SetStopCriteria( criteria, &critParam );
}

TSP::~TSP()
{
	delete _algorithm;

	delete _population;
	delete _populationConfiguration;

	delete _prototype;

	delete &_ccb->GetCrossoverOperation();
	delete &_ccb->GetFitnessOperation();
	delete &_ccb->GetParameters();
	delete _ccb;

	GaFinalize();
}