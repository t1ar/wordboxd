#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <string>

class Movie; // forward declaration agar bisa digunakan di class lain sebelum didefinisikan

// custom linked list untuk string, dibuat manual tanpa std::vector
class StringNode {
public:
    std::string data;
    StringNode* next; // pointer ke node string berikutnya
    StringNode(std::string d) : data(d), next(nullptr) {}
};

class StringList {
public:
    StringNode* head;
    StringNode* tail;
    int count;

    StringList() : head(nullptr), tail(nullptr), count(0) {}
    
    // menambahkan string baru di akhir linked list
    void add(std::string d) {
        StringNode* n = new StringNode(d);
        if (!head) { head = tail = n; }
        else { tail->next = n; tail = n; }
        count++;
    }
    
    // mengambil nilai string pada indeks tertentu
    std::string get(int index) {
        StringNode* curr = head;
        for(int i = 0; i < index && curr != nullptr; i++) curr = curr->next;
        return curr ? curr->data : "";
    }
    
    bool empty() { return count == 0; }
};

// custom linked list khusus untuk menyimpan pointer objek movie (berguna untuk relasi graf)
class MovieListNode {
public:
    Movie* movie;
    MovieListNode* next;
    MovieListNode(Movie* m) : movie(m), next(nullptr) {}
};

class MovieList {
public:
    MovieListNode* head;
    MovieListNode* tail;
    int count;

    MovieList() : head(nullptr), tail(nullptr), count(0) {}
    
    // menambahkan pointer movie ke akhir list
    void add(Movie* m) {
        MovieListNode* n = new MovieListNode(m);
        if (!head) { head = tail = n; }
        else { tail->next = n; tail = n; }
        count++;
    }
    
    // mengambil pointer movie berdasarkan indeks
    Movie* get(int index) {
        MovieListNode* curr = head;
        for(int i = 0; i < index && curr != nullptr; i++) curr = curr->next;
        return curr ? curr->movie : nullptr;
    }
    
    bool empty() { return count == 0; }
};

// struktur data utama untuk menyimpan informasi film
class Movie {
public:
    std::string nama;
    StringList genre_names; // menyimpan daftar nama genre film ini
    int ep;
    int season;
    std::string studio;
    int total_rating;
    float rating;
    
    // implementasi graph (adjacency list). menyimpan kumpulan film lain yang berelasi dengan film ini.
    // ini membentuk graf tak berarah (undirected graph) di mana node adalah film, dan edge adalah relasinya.
    MovieList related_movies; 
    
    // pointer untuk struktur binary search tree (bst)
    // left akan menunjuk ke node movie dengan nama (string) yang lebih kecil
    // right akan menunjuk ke node movie dengan nama (string) yang lebih besar
    Movie* left;
    Movie* right;

    Movie(std::string n, int e, int s, std::string st) 
        : nama(n), ep(e), season(s), studio(st), total_rating(0), rating(0.0f), left(nullptr), right(nullptr) {}

    // menghitung rata-rata rating baru berdasarkan input user
    void addRating(float new_rating) {
        rating = ((rating * total_rating) + new_rating) / (total_rating + 1);
        total_rating++;
    }
};

// node khusus untuk tabel hash (hash table) yang menyimpan genre
class GenreNode {
public:
    std::string nama_genre;
    MovieList movies; // daftar film yang masuk ke dalam genre ini
    
    // pointer next digunakan untuk chaining dalam hash table.
    // jika ada dua genre yang hasil hash-nya sama (collision), node baru akan disambung ke sini.
    GenreNode* next;  

    GenreNode(std::string n) : nama_genre(n), next(nullptr) {}
};

#endif