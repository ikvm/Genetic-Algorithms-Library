
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#include "StdAfx.h"
#include "TspCity.h"

#include <math.h>

int TspCity::_nextId = 0;

float TspCity::GetDistance(const TspCity& city) const
{
	int s = abs( _x - city._x );
	int v = abs( _y - city._y );

	return sqrt( (float)( s * s + v * v ) );
}

void TspCity::Draw(CDC& dc) const
{
	dc.Ellipse( _x - CITY_SIZE / 2, _y - CITY_SIZE / 2, _x + CITY_SIZE / 2, _y + CITY_SIZE / 2 );
	dc.TextOutW( _x - 5, _y - CITY_SIZE / 2 - 17, _name.c_str(), (int)_name.length() );
}

void TspCities::AddCity(const wstring& name,
			 int x,
			 int y)
{
	TspCity* _newCity = new TspCity( name, x, y );
	_cities.insert( pair<int, TspCity*>( _newCity->GetID(), _newCity ) );
}

TspCities TspCities::_instance;

bool TspCities::RemoveCity(int id)
{
	TspCity* city = _cities[ id ];
	if( city )
	{
		_cities.erase( id );
		delete city;

		return true;
	}

	return false;
}

void TspCities::Clear()
{
	for( hash_map<int, TspCity*>::iterator it = _cities.begin(); it != _cities.end(); ++it )
		delete it->second;

	_cities.clear();
}

void TspCities::DrawCities(CDC& dc) const
{
	for( hash_map<int, TspCity*>::const_iterator it = _cities.begin(); it != _cities.end(); ++it )
		it->second->Draw( dc );
}

float TspCities::GetDistance(int cityA, int cityB) const
{
	const TspCity* a = GetCityById( cityA );
	if( !a )
		return -1;

	const TspCity* b = GetCityById( cityB );
	if( !b )
		return -1;

	return a->GetDistance( *b );
}

const TspCity* TspCities::GetCityById(int id) const
{
	hash_map<int, TspCity*>::const_iterator it = _cities.find( id );
	if( it != _cities.end() )
		return it->second;

	return NULL;
}

const TspCity* TspCities::GetCityByPoint(int x,
										 int y) const
{
	for( hash_map<int, TspCity*>::const_iterator it = _cities.begin(); it != _cities.end(); ++it )
	{
		if( it->second->PointWithinCity( x, y ) )
			return it->second;
	}

	return NULL;
}

void TspCities::GetCities(vector<const TspCity*>& output) const
{
	output.resize( _cities.size() );

	int i = 0;
	for( hash_map<int, TspCity*>::const_iterator it = _cities.begin(); it != _cities.end(); ++it, ++i )
		output[ i ] = it->second;
}
