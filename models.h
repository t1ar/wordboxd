#ifndef MODELS_H
#define MODELS_H

#include <iostream>
#include <string>

class Movie; // Forward declaration

// Custom Linked List untuk String
class StringNode {
public:
    std::string data;
    StringNode* next;
    StringNode(std::string d) : data(d), next(nullptr) {}
};

class StringList {
public:
    StringNode* head;
    StringNode* tail;
    int count;

    StringList() : head(nullptr), tail(nullptr), count(0) {}
    
    void add(std::string d) {
        StringNode* n = new StringNode(d);
        if (!head) { head = tail = n; }
        else { tail->next = n; tail = n; }
        count++;
    }
    
    std::string get(int index) {
        StringNode* curr = head;
        for(int i = 0; i < index && curr != nullptr; i++) curr = curr->next;
        return curr ? curr->data : "";
    }
    
    bool empty() { return count == 0; }
};

// Custom Linked List untuk Pointer Movie
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
    
    void add(Movie* m) {
        MovieListNode* n = new MovieListNode(m);
        if (!head) { head = tail = n; }
        else { tail->next = n; tail = n; }
        count++;
    }
    
    Movie* get(int index) {
        MovieListNode* curr = head;
        for(int i = 0; i < index && curr != nullptr; i++) curr = curr->next;
        return curr ? curr->movie : nullptr;
    }
    
    bool empty() { return count == 0; }
};

class Movie {
public:
    std::string nama;
    StringList genre_names; // Menggantikan vector string
    int ep;
    int season;
    std::string studio;
    int total_rating;
    float rating;
    
    MovieList related_movies; // Menggantikan vector Movie* untuk graf
    
    // Pointer untuk Binary Search Tree
    Movie* left;
    Movie* right;

    Movie(std::string n, int e, int s, std::string st) 
        : nama(n), ep(e), season(s), studio(st), total_rating(0), rating(0.0f), left(nullptr), right(nullptr) {}

    void addRating(float new_rating) {
        rating = ((rating * total_rating) + new_rating) / (total_rating + 1);
        total_rating++;
    }
};

class GenreNode {
public:
    std::string nama_genre;
    MovieList movies; // Kumpulan film dengan genre ini (Graf unweighted)
    GenreNode* next;  // Chaining untuk Hash Table

    GenreNode(std::string n) : nama_genre(n), next(nullptr) {}
};

#endif