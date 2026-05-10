#ifndef DATABASE_H
#define DATABASE_H

#include "Models.h"
#include <fstream> // Tambahkan untuk operasi file

class Database {
private:
    Movie* root;
    static const int HASH_SIZE = 10;
    GenreNode* genreTable[HASH_SIZE];

    int hashFunction(std::string key);
    Movie* insertBST(Movie* node, Movie* newMovie);
    Movie* searchBST(Movie* node, std::string name);
    void getMoviesFiltered(Movie* node, bool isSeries, MovieList& result);
    
    // Helper untuk menyimpan data secara rekursif (In-order Traversal)
    void saveRecursive(Movie* node, std::ofstream& file);

public:
    Database();
    ~Database();
    
    void initDefaultGenres();
    GenreNode* getGenre(std::string name);
    
    bool addMovie(std::string name, StringList genres, int ep, int season, StringList related, std::string studio, bool silent = false);
    Movie* searchMovie(std::string name);
    MovieList getAll(bool isSeries);

    // Fungsi Database Fisik
    void saveToFile();
    void loadFromFile();
};

#endif