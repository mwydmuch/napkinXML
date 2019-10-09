/**
 * Copyright (c) 2018 by Marek Wydmuch
 * All rights reserved.
 */

#pragma once

#include <cstdlib>
#include <cstddef>
#include <cassert>
#include <cstring>
#include <vector>
#include <fstream>

#include "linear.h"

typedef int Label;
typedef int Example;
typedef feature_node DoubleFeature;
typedef DoubleFeature Feature;

struct IntFeature{
    int index;
    int value;

    bool operator<(const IntFeature &r) const { return value < r.value; }

    friend std::ostream& operator<<(std::ostream& os, const IntFeature& f) {
        os << f.index << ":" << f.value;
        return os;
    }
};

// Dense matrix
template <typename T>
class DMatrix {
    DMatrix();
    DMatrix(int rows, int cols);
    ~DMatrix();
};

// Elastic low-level sparse row matrix, type T needs to contain int at offset 0!
template <typename T>
class SRMatrix {
public:
    SRMatrix();
    SRMatrix(int rows, int cols);
    ~SRMatrix();

    inline void appendRow(const std::vector<T>& row);
    void appendRow(const T* row, const int size);

    inline void replaceRow(const int index, const std::vector<T>& row);
    void replaceRow(const int index, const T* row, const int size);

    // Inefficient operation
    void appendToRow(const int index, const std::vector<T>& row);
    void appendToRow(const int index, const T* data, const int size = 1);

    // Row multiplication
    template <typename U>
    inline U dotRow(const int index, const std::vector<U>& vector);

    template <typename U>
    inline U dotRow(const int index, const U* vector);

    template <typename U>
    inline U dotRow(const int index, const U* vector, const int size);

    // Returns data as T**
    inline T** data(){ return r.data(); }
    //inline const T** data() const { return r.data(); }

    // Returns row as T*
    //inline T* row(const int index) { return r[index]; }
    inline T* row(const int index) const { return r[index]; }

    // Returns std::vector<T*>&
    inline std::vector<T*>& allRows() { return r; }

    // Access row also by [] operator
    inline T& operator[](const int index) { return r[index]; }
    inline const T& operator[] (const int index) const { return r[index]; }

    // Returns rows' sizes
    inline std::vector<int>& sizes(){ return s; }

    // Returns single row size
    inline int size(const int index) const { return s[index]; }

    // Returns transposed matrix
    SRMatrix<T> transopose();

    // Returns size of matrix
    inline int rows() const { return m; }
    inline int cols() const { return n; }
    inline int cells() const { return c; }

    void clear();
    void saveToFile(std::string outfile);
    void save(std::ostream& out);
    void saveAsTextFile(std::string outfile);
    void loadFromFile(std::string infile);
    void load(std::istream& in);

private:
    int m; // Row count
    int n; // Col count
    int c; // Non zero cells count
    std::vector<int> s; // Rows' sizes
    std::vector<T*> r; // Rows

    inline T* createNewRow(const T* row, const int size);
    inline void updateN(const T* row, const int size);
};

template <typename T>
SRMatrix<T>::SRMatrix(){
    m = 0;
    n = 0;
    c = 0;
}

template <typename T>
SRMatrix<T>::SRMatrix(int rows, int cols){
    m = rows;
    n = cols;
    c = m * n;

    for(int i = 0; i < m; ++i){
        s.push_back(n);
        T* newRow = new T[n + 1];
        for(int j = 0; j < n; ++j) std::memset(&newRow[j], j, sizeof(int)); // Set next index
        std::memset(&newRow[n], -1, sizeof(int)); // Add termination feature (-1)
        r.push_back(newRow);
    }
}

template <typename T>
SRMatrix<T>::~SRMatrix(){
    clear();
}

template <typename T>
inline T* SRMatrix<T>::createNewRow(const T* row, const int size){
    T* newRow = new T[size + 1];
    std::memcpy(newRow, row, size * sizeof(T));
    std::memset(&newRow[size], -1, sizeof(int)); // Add termination feature (-1)
    return newRow;
}

template <typename T>
inline void SRMatrix<T>::updateN(const T* row, const int size){
    if(size > 0){
        int rown = *(int *)&row[size - 1] + 1;
        if(n < rown) n = rown;
    }
}

// Data should be sorted
template <typename T>
inline void SRMatrix<T>::appendRow(const std::vector<T>& row){
    appendRow(row.data(), row.size());
}

template <typename T>
void SRMatrix<T>::appendRow(const T* row, const int size){
    s.push_back(size);
    r.push_back(createNewRow(row, size));
    updateN(row, size);
    m = r.size();
    c += size;
}

// Data should be sorted
template <typename T>
inline void SRMatrix<T>::replaceRow(const int index, const std::vector<T>& row){
    replaceRow(index, row.data(), row.size());
}

template <typename T>
void SRMatrix<T>::replaceRow(const int index, const T* row, const int size){
    c += size - s[index];
    s[index] = size;
    delete[] r[index];
    r[index] = createNewRow(row, size);
    updateN(row, size);
}

template <typename T>
inline void SRMatrix<T>::appendToRow(const int index, const std::vector<T>& data){
    appendToRow(index, data.data(), data.size());
}

template <typename T>
inline void SRMatrix<T>::appendToRow(const int index, const T* data, const int size){
    int rSize = s[index];
    T* newRow = new T[s[index] + size + 1];
    std::memcpy(newRow, r[index], rSize * sizeof(T));
    std::memcpy(newRow + rSize, data, size * sizeof(T));
    std::memset(&newRow[size], -1, sizeof(int)); // Add termination feature (-1)
    delete[] r[index];
    r[index] = newRow;
    s[index] += size;
    c += size;
}

template <typename T>
template <typename U>
inline U SRMatrix<T>::dotRow(const int index, const std::vector<U>& vector){
    return dotVectors(r[index], vector.data(), vector.size());
}

template <typename T>
template <typename U>
inline U SRMatrix<T>::dotRow(const int index, const U* vector, const int size){
    return dotVectors(r[index], vector, size);
}

template <typename T>
template <typename U>
inline U SRMatrix<T>::dotRow(const int index, const U* vector){ // Version without size checks
    return dotVectors(r[index], vector);
}

// TODO
template <typename T>
inline SRMatrix<T> SRMatrix<T>::transopose() {
    SRMatrix<T> tMatrix;
    return tMatrix;
}


template <typename T>
void SRMatrix<T>::clear(){
    for(auto row : r) delete[] row;
    r.clear();
    s.clear();

    m = 0;
    n = 0;
    c = 0;
}

template <typename T>
void SRMatrix<T>::saveToFile(std::string outfile){
    std::ofstream out(outfile);
    save(out);
    out.close();
}

template <typename T>
void SRMatrix<T>::save(std::ostream& out){
    out << m << " " << n << "\n";
    for(int i = 0; i < m; ++i){
        out << s[i];
        for(int j = 0; j < s[i]; ++j)
            out << " " << r[i][j];
        out << "\n";
    }
}

template <typename T>
void SRMatrix<T>::saveAsTextFile(std::string outfile) {
    std::ofstream out(outfile);
    out << m << " " << n << "\n";
    for (int i = 0; i < m; ++i) {
        out << s[i];
        for (int j = 0; j < s[i]; ++j)
            out << " " << r[i][j];
        out << "\n";
    }
    out.close();
}

template <typename T>
void SRMatrix<T>::loadFromFile(std::string infile){
    std::ifstream in(infile);
    load(in);
    in.close();
}

template <typename T>
void SRMatrix<T>::load(std::istream& in) {
    clear();

    in.read((char*) &m, sizeof(m));
    in.read((char*) &n, sizeof(n));

    r.resize(m);
    s.resize(m);

    for(int i = 0; i < m; ++i) {
        int size;
        in.read((char*) &size, sizeof(size));
        T* newRow = new T[size + 1];
        s[i] = size;
        r[i] = newRow;
        c += size;

        for (int j = 0; j <= size; ++j)
            in.read((char *) &r[i][j], sizeof(T));
    }
}
