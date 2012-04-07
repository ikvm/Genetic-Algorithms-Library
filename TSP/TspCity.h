
/*
 * 
 * website: N/A
 * contact: kataklinger@gmail.com
 *
 */

#pragma once

#include <string>
#include <hash_map>

using namespace std;
using namespace stdext;

#define CITY_SIZE 14

class TspCity
{

private:

	static int _nextId;

	int _id;

	wstring _name;

	int _x;

	int _y;

public:

	TspCity(const wstring& name,
		int x,
		int y) : _name(name),
		_x(x),
		_y(y) { _id = ++_nextId; }

	inline int GetID() const { return _id; }

	inline int GetX() const { return _x; }

	inline int GetY() const { return _y; }

	float GetDistance(const TspCity& city) const;

	inline const wstring& GetName() const { return _name; }

	inline bool PointWithinCity(int x, int y) { return	x >= _x - CITY_SIZE / 2 && x <= _x + CITY_SIZE / 2 && y >= _y - CITY_SIZE / 2 && y <= _y + CITY_SIZE / 2; }

	void Draw(CDC& dc) const;

};

class TspCities
{

private:

	static TspCities _instance;

	hash_map<int, TspCity*> _cities;

public:

	inline static TspCities& GetInstance() { return _instance; }

	~TspCities() { Clear(); }

	void AddCity(const wstring& name,
		int x,
		int y);

	bool RemoveCity(int id);

	void Clear();

	void DrawCities(CDC& dc) const;

	float GetDistance(int cityA, int cityB) const;

	const TspCity* GetCityById(int id) const;

	const TspCity* GetCityByPoint(int x,
		int y) const;

	void GetCities(vector<const TspCity*>& output) const;

	inline int GetCount() const { return (int)_cities.size(); }

};
