#pragma once


#include "data.h"
#include <string>
#include <vector>

 // ─────────────────────────────────────────────
 //  Result / Error types
 // ─────────────────────────────────────────────

 // Possible outcomes of a logic operation
enum class LogicResult
{
    OK = 0,
    ERR_NOT_FOUND = 1,   // Student / record not found
    ERR_INVALID = 2,   // Invalid input data
    ERR_DUPLICATE = 3,   // Duplicate entry
    ERR_IO = 4,   // File I/O failure
    ERR_EMPTY = 5    // Empty data set
};

// Human-readable description of a LogicResult
std::string logic_resultMessage(LogicResult result);

// ─────────────────────────────────────────────
//  Sort options
// ─────────────────────────────────────────────

// How to sort a list of students
enum class SortField
{
    BY_LAST_NAME = 0,    // Alphabetically by last name
    BY_AVERAGE = 1,    // By average grade (descending by default)
    BY_CLASS = 2,    // By class label
    BY_ID = 3     // By student ID
};

enum class SortOrder
{
    ASCENDING = 0,
    DESCENDING = 1
};

// ─────────────────────────────────────────────
//  Statistics structures
// ─────────────────────────────────────────────

// Aggregated stats for a single subject across a group of students
struct SubjectStats
{
    Subject     subject;
    double      average;      // Arithmetic mean
    double      highest;      // Maximum grade
    double      lowest;       // Minimum grade
    int         count;        // Number of grades entered
    int         failing;      // Students with grade < 3.0
};

// Full statistics for a class or the whole school
struct GroupStats
{
    std::string groupName;            // Class label or "Всички"
    double      overallAverage;       // Average of all grades
    int         totalStudents;        // Number of active students
    int         totalFailing;         // Students with ANY grade < 3.0
    int         excellentCount;       // Students with average >= 5.50
    std::vector<SubjectStats> bySubject;
};

// ─────────────────────────────────────────────
//  Function Prototypes – Sorting
// ─────────────────────────────────────────────

// Sort a vector of student pointers in-place using Bubble Sort
// Returns the number of swaps performed (for visualisation)
int logic_bubbleSort(std::vector<Student*>& students,
    SortField field,
    SortOrder order = SortOrder::ASCENDING);

// Sort a vector of student pointers in-place using Quick Sort
void logic_quickSort(std::vector<Student*>& students,
    SortField field,
    SortOrder order = SortOrder::ASCENDING,
    int low = 0,
    int high = -1);   // -1 means "use vector size - 1"

// Sort grades within a single student by subject enum value
void logic_sortStudentGrades(Student& student);

// ─────────────────────────────────────────────
//  Function Prototypes – Searching
// ─────────────────────────────────────────────

// Linear search: find all students whose lastName starts with prefix
// Returns matched students (case-insensitive)
std::vector<Student*> logic_linearSearchByName(DataStore& store,
    const std::string& prefix);

// Binary search on a SORTED (by last name) vector; returns index or -1
int logic_binarySearchByName(const std::vector<Student*>& sorted,
    const std::string& lastName);

// Find the student with the highest average grade in a group
Student* logic_findTopStudent(const std::vector<Student*>& students);

// Find the student with the lowest average grade in a group
Student* logic_findBottomStudent(const std::vector<Student*>& students);

// Find all students whose average grade is within [minGrade, maxGrade]
std::vector<Student*> logic_filterByAverageRange(
    const std::vector<Student*>& students,
    double minGrade,
    double maxGrade);

// Find all students who are failing (have at least one grade < 3.0)
std::vector<Student*> logic_findFailing(const std::vector<Student*>& students);

// ─────────────────────────────────────────────
//  Function Prototypes – Recursive Computations
// ─────────────────────────────────────────────

// Recursively calculate the average grade of a student
// across all entered subjects.
// Base case: 0 subjects → returns 0.0
double logic_recursiveAverage(const std::vector<SubjectGrade>& grades,
    int index = 0,
    double accumulator = 0.0);

// Recursively compute the sum of averages for a list of students
// Used internally; base case: empty list → 0.0
double logic_recursiveSumAverages(const std::vector<Student*>& students,
    int index = 0);

// Recursively count students who are failing in a group
// (have at least one grade below 3.0)
int logic_recursiveCountFailing(const std::vector<Student*>& students,
    int index = 0);

// Recursively find the student with the maximum average in [lo, hi]
// Returns the index in the vector
int logic_recursiveFindMax(const std::vector<Student*>& students,
    int lo, int hi);

// Recursively generate a Fibonacci number (for demonstration purposes)
// fibonacci(0)=0, fibonacci(1)=1, fibonacci(n)=fibonacci(n-1)+fibonacci(n-2)
long long logic_fibonacci(int n);

// Recursively calculate n! (factorial) – used in permutation stats
long long logic_factorial(int n);

// ─────────────────────────────────────────────
//  Function Prototypes – Averages & Statistics
// ─────────────────────────────────────────────

// Compute the average grade for a single student (non-recursive)
double logic_studentAverage(const Student& student);

// Compute statistics for a group of student pointers
GroupStats logic_groupStats(const std::vector<Student*>& students,
    const std::string& groupName);

// Compute statistics for a specific class
GroupStats logic_classStats(DataStore& store, const std::string& classLabel);

// Compute statistics for the whole school
GroupStats logic_schoolStats(DataStore& store);

// Return a formatted grade string: "5.50" → displayed as "Отличен (5.50)"
std::string logic_gradeLabel(double average);

// Return a color code index for a grade (for UI rendering)
// 0=red(Слаб), 1=orange(Среден), 2=yellow(Добър),
// 3=light-green(Много добър), 4=green(Отличен)
int logic_gradeColorIndex(double grade);

// ─────────────────────────────────────────────
//  Function Prototypes – Student Management
// ─────────────────────────────────────────────

// Validate all required fields of a student before adding/updating
LogicResult logic_validateStudent(const Student& student);

// Add a new student through the logic layer (validates first)
LogicResult logic_addStudent(DataStore& store, Student& student,
    int& outId);

// Update an existing student's personal data
LogicResult logic_updateStudent(DataStore& store, const Student& student);

// Delete a student (soft delete)
LogicResult logic_deleteStudent(DataStore& store, int id);

// Add or update a grade, with validation
LogicResult logic_setGrade(DataStore& store, int studentId,
    Subject subject, double gradeValue,
    const std::string& note = "");

// Remove a grade
LogicResult logic_removeGrade(DataStore& store, int studentId, Subject subject);

// ─────────────────────────────────────────────
//  Function Prototypes – Persistence (through logic)
// ─────────────────────────────────────────────

LogicResult logic_save(const DataStore& store, const std::string& filepath);
LogicResult logic_load(DataStore& store, const std::string& filepath);

// ─────────────────────────────────────────────
//  Function Prototypes – Ranking
// ─────────────────────────────────────────────

// Build a ranked list of students (sorted by average, descending)
// Returns the sorted list; position in vector = rank-1
std::vector<Student*> logic_buildRanking(DataStore& store,
    const std::string& classFilter = "");
// ─────────────────────────────────────────────
//  Authentication
// ─────────────────────────────────────────────

/*
 
Attempt login with the given username and password.
Returns a pointer to the matched User on success, nullptr on failure.
The password is hashed before comparison – plain text is never stored.*/
const User* logic_login(const UserStore& users,
    const std::string& username,
    const std::string& password);

// Returns true if the logged-in user is allowed to modify data
bool logic_canEdit(const User* user);

// Returns true if the logged-in user is allowed to delete records
bool logic_canDelete(const User* user);
