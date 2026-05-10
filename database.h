#ifndef DATABASE_H
#define DATABASE_H

#include "Models.h"
#include <fstream> // tambahkan untuk operasi file

class Database {
private:
    // root adalah simpul akar atau titik awal dari binary search tree (bst)
    Movie* root;
    
    // ukuran tabel hash yang ditentukan secara statis
    static const int HASH_SIZE = 10;
    
    // array dari pointer untuk menyimpan hash table dari genre
    GenreNode* genreTable[HASH_SIZE];

    // deklarasi fungsi internal untuk algoritma data structure
    int hashFunction(std::string key);
    Movie* insertBST(Movie* node, Movie* newMovie);
    Movie* searchBST(Movie* node, std::string name);
    void getMoviesFiltered(Movie* node, bool isSeries, MovieList& result);
    
    // helper untuk menyimpan data secara rekursif (in-order traversal)
    void saveRecursive(Movie* node, std::ofstream& file);

public:
    Database();
    ~Database();
    
    void initDefaultGenres();
    GenreNode* getGenre(std::string name);
    
    bool addMovie(std::string name, StringList genres, int ep, int season, StringList related, std::string studio, bool silent = false);
    Movie* searchMovie(std::string name);
    MovieList getAll(bool isSeries);

    // fungsi database fisik
    void saveToFile();
    void loadFromFile();
};

#endif