// DataTest.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <chrono>



using namespace std;
using namespace std::chrono;


struct TestData {
  int id;
  string text;

  TestData(int h, string m) : id(h), text(m) {}
  TestData(){};
};

// Returns true if t1 < t2
bool operator<(const TestData& ba1, const TestData& ba2)
{
  return ba1.id < ba2.id;
}

int identifier;

int main()
{
  // Crear variables

  vector<TestData> v;
  list<TestData> l;
  map<int,TestData> m;
  unordered_map<int,TestData> um;
  //////////////////////////////////////////////
  cout << "Insertar" << endl;
  cout << "---------------------------------------------" << endl;

  ////////////////////////////////////////////
  // Llenado de vector
  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  for(int i=0; i<10000;i++){
    TestData hola(i,"prueba");
    v.push_back(hola);
  }
  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  auto InsertVect = duration_cast<microseconds>( t2 - t1 ).count();
  cout << "Vector " << InsertVect << endl;

  ////////////////////////////////////////////
  //Llenado de Lista
  t1 = high_resolution_clock::now();
  for(int i=0; i<10000;i++){
    TestData hola(i,"prueba");
    l.push_back(hola);
  }
  t2 = high_resolution_clock::now();
  auto InsertList = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Lista " << InsertList << endl;



  ////////////////////////////////////////////
  //Llenado de Map
  t1 = high_resolution_clock::now();
  for(int i=0; i<10000;i++){
    TestData hola(i,"prueba");
    m.emplace(i, hola);
  }
  t2 = high_resolution_clock::now();
  auto Insertmap = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Map " << Insertmap << endl;

  ////////////////////////////////////////////
  //Llenado de unmap
  t1 = high_resolution_clock::now();
  for(int i=0; i<10000;i++){
    TestData hola(i,"prueba");
    um.emplace (i, hola);
  }
  t2 = high_resolution_clock::now();
  auto Insertunmap = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Multimap " << Insertunmap << endl;




    //////////////////////////////////////////////
  cout << endl << "Seleccionar un id" << endl;
  cout << "---------------------------------------------" << endl;

  ////////////////////////////////////////////
  // Llenado de vector
  t1 = high_resolution_clock::now();
  auto temp = v[7430];
  t2 = high_resolution_clock::now();
  auto InsertVect2 = duration_cast<microseconds>( t2 - t1 ).count();
  cout << "Vector " << InsertVect2 << endl;

  ////////////////////////////////////////////
  //Llenado de Lista
  int i= l.size();
  t1 = high_resolution_clock::now();
  for (auto it=l.end() ; it != l.begin(); --it){
    --i;
    if(i == 7430){
      auto temp = *it;
    }
  }
  t2 = high_resolution_clock::now();
  auto InsertList2 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Lista " << InsertList2 << endl;



  ////////////////////////////////////////////
  //Llenado de Map
  t1 = high_resolution_clock::now();
  auto temp1 = m[7430];
  t2 = high_resolution_clock::now();
  auto Insertmap2 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Map " << Insertmap2 << endl;

  ////////////////////////////////////////////
  //Llenado de unmap
  t1 = high_resolution_clock::now();
  auto temp3 = um[7430];
  t2 = high_resolution_clock::now();
  auto Insertunmap2 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Multimap " << Insertunmap2 << endl;



    //////////////////////////////////////////////
  cout << endl << "Borrar un id" << endl;
  cout << "---------------------------------------------" << endl;

  ////////////////////////////////////////////
  // Llenado de vector
  t1 = high_resolution_clock::now();
  v.erase(v.begin()+7430);
  t2 = high_resolution_clock::now();
  auto InsertVect3 = duration_cast<microseconds>( t2 - t1 ).count();
  cout << "Vector " << InsertVect3 << endl;

  ////////////////////////////////////////////
  //Llenado de Lista

  int ii= l.size();
  t1 = high_resolution_clock::now();
  for (auto it=l.end() ; it != l.begin(); --it){
    --ii;
    if(ii == 7430){
      it = l.erase(it);
    }
  }
  t2 = high_resolution_clock::now();
  auto InsertList3 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Lista " << InsertList3 << endl;



  ////////////////////////////////////////////
  //Llenado de Map
  t1 = high_resolution_clock::now();
  std::map<int,TestData>::iterator itmap;
  itmap = m.find(7430);
  m.erase(itmap);
  t2 = high_resolution_clock::now();
  auto Insertmap3 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Map " << Insertmap3 << endl;

  ////////////////////////////////////////////
  //Llenado de unmap
  t1 = high_resolution_clock::now();
  um.erase(7430);
  t2 = high_resolution_clock::now();
  auto Insertunmap3 = duration_cast<microseconds>( t2 - t1 ).count();

  cout << "Multimap " << Insertunmap3 << endl;
  cout << endl << "END" << endl;

  return 0;
}