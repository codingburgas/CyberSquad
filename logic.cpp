
#include "logic.h"
#include "data.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>

 // ═══════════════════════════════════════════════════════════════
 //  Internal helpers
 // ═══════════════════════════════════════════════════════════════

 // Case-insensitive compare of two strings
static bool ciLess(const std::string& a, const std::string& b)
{
    std::string la = a, lb = b;
    std::transform(la.begin(), la.end(), la.begin(), ::tolower);
    std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
    return la < lb;
}

// Compare two Student pointers based on SortField and SortOrder
static bool studentCompare(const Student* a, const Student* b,
    SortField field, SortOrder order)
{
    bool result = false;

    switch (field)
    {
    case SortField::BY_LAST_NAME:
        result = ciLess(a->lastName, b->lastName);
        break;

    case SortField::BY_AVERAGE:
    {
        double avgA = logic_studentAverage(*a);
        double avgB = logic_studentAverage(*b);
        result = avgA < avgB;
        break;
    }

    case SortField::BY_CLASS:
        result = ciLess(a->classLabel, b->classLabel);
        break;

    case SortField::BY_ID:
        result = a->id < b->id;
        break;
    }

    // Flip for descending order
    return (order == SortOrder::ASCENDING) ? result : !result;
}

// ═══════════════════════════════════════════════════════════════
//  Result messages
// ═══════════════════════════════════════════════════════════════

std::string logic_resultMessage(LogicResult result)
{
    switch (result)
    {
    case LogicResult::OK:            return "Operation successful.";
    case LogicResult::ERR_NOT_FOUND: return "Record not found.";
    case LogicResult::ERR_INVALID:   return "Invalid input data.";
    case LogicResult::ERR_DUPLICATE: return "Record already exists.";
    case LogicResult::ERR_IO:        return "File read/write error.";
    case LogicResult::ERR_EMPTY:     return "No data available.";
    default:                         return "Unknown error.";
    }
}

// ═══════════════════════════════════════════════════════════════
//  Sorting – Bubble Sort
// ═══════════════════════════════════════════════════════════════

/*
 * logic_bubbleSort
 * Classic O(n²) Bubble Sort with early-exit optimisation.
 * Returns the total number of swaps performed.
 */
int logic_bubbleSort(std::vector<Student*>& students,
    SortField field,
    SortOrder order)
{
    int n = static_cast<int>(students.size());
    int swaps = 0;

    for (int i = 0; i < n - 1; ++i)
    {
        bool swapped = false;

        for (int j = 0; j < n - i - 1; ++j)
        {
            // If students[j] should come AFTER students[j+1], swap them
            if (!studentCompare(students[j], students[j + 1], field, order))
            {
                std::swap(students[j], students[j + 1]);
                ++swaps;
                swapped = true;
            }
        }

        // Early exit if no swaps happened in this pass
        if (!swapped) break;
    }

    return swaps;
}

// ═══════════════════════════════════════════════════════════════
//  Sorting – Quick Sort
// ═══════════════════════════════════════════════════════════════

/*
 * Partition helper for Quick Sort.
 * Uses the last element as pivot.
 * Returns the final position of the pivot.
 */
static int quickSortPartition(std::vector<Student*>& students,
    SortField field,
    SortOrder order,
    int low, int high)
{
    Student* pivot = students[high];
    int i = low - 1;

    for (int j = low; j < high; ++j)
    {
        if (studentCompare(students[j], pivot, field, order))
        {
            ++i;
            std::swap(students[i], students[j]);
        }
    }

    std::swap(students[i + 1], students[high]);
    return i + 1;
}

/*
 * logic_quickSort
 * Recursive O(n log n) average Quick Sort.
 * On the first call from outside, pass high = -1 to auto-set it.
 */
void logic_quickSort(std::vector<Student*>& students,
    SortField field,
    SortOrder order,
    int low,
    int high)
{
    // Auto-detect high on first call
    if (high == -1)
        high = static_cast<int>(students.size()) - 1;

    if (low < high)
    {
        int pi = quickSortPartition(students, field, order, low, high);
        logic_quickSort(students, field, order, low, pi - 1);
        logic_quickSort(students, field, order, pi + 1, high);
    }
}

/*
 * logic_sortStudentGrades
 * Sort the grades list inside a student by subject enum value.
 */
void logic_sortStudentGrades(Student& student)
{
    std::sort(student.grades.begin(), student.grades.end(),
        [](const SubjectGrade& a, const SubjectGrade& b)
        {
            return static_cast<int>(a.subject) < static_cast<int>(b.subject);
        });
}

// ═══════════════════════════════════════════════════════════════
//  Searching
// ═══════════════════════════════════════════════════════════════

/*
 * logic_linearSearchByName
 * O(n) scan of all active students.
 * Returns all students whose lastName starts with the given prefix
 * (case-insensitive).
 */
std::vector<Student*> logic_linearSearchByName(DataStore& store,
    const std::string& prefix)
{
    std::vector<Student*> results;
    if (prefix.empty()) return results;

    std::string lprefix = prefix;
    std::transform(lprefix.begin(), lprefix.end(), lprefix.begin(), ::tolower);

    std::vector<Student*> all = data_getAllActive(store);
    for (Student* s : all)
    {
        std::string lname = s->lastName;
        std::transform(lname.begin(), lname.end(), lname.begin(), ::tolower);

        if (lname.find(lprefix) == 0) // Starts with prefix
            results.push_back(s);
    }

    return results;
}

/*
 * logic_binarySearchByName
 * Binary search on a vector ALREADY SORTED by last name.
 * Returns the index of the first match or -1 if not found.
 * Search is case-insensitive and exact on the full last name.
 */
int logic_binarySearchByName(const std::vector<Student*>& sorted,
    const std::string& lastName)
{
    if (sorted.empty()) return -1;

    std::string query = lastName;
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    int lo = 0;
    int hi = static_cast<int>(sorted.size()) - 1;

    while (lo <= hi)
    {
        int mid = lo + (hi - lo) / 2;
        std::string mid_name = sorted[mid]->lastName;
        std::transform(mid_name.begin(), mid_name.end(),
            mid_name.begin(), ::tolower);

        if (mid_name == query)
        {
            // Walk back to find the first occurrence
            while (mid > 0)
            {
                std::string prev = sorted[mid - 1]->lastName;
                std::transform(prev.begin(), prev.end(), prev.begin(), ::tolower);
                if (prev == query) --mid;
                else break;
            }
            return mid;
        }
        else if (mid_name < query)
            lo = mid + 1;
        else
            hi = mid - 1;
    }

    return -1; // Not found
}

/*
 * logic_findTopStudent
 * Linear scan; returns the student with the highest average.
 * Ties broken by student ID (lower ID wins).
 */
Student* logic_findTopStudent(const std::vector<Student*>& students)
{
    if (students.empty()) return nullptr;

    Student* top = nullptr;
    double   topAvg = -1.0;

    for (Student* s : students)
    {
        double avg = logic_studentAverage(*s);
        if (avg > topAvg)
        {
            topAvg = avg;
            top = s;
        }
    }

    return top;
}

/*
 * logic_findBottomStudent
 * Returns the student with the lowest average (among those with any grades).
 */
Student* logic_findBottomStudent(const std::vector<Student*>& students)
{
    if (students.empty()) return nullptr;

    Student* bottom = nullptr;
    double   bottomAvg = 7.0; // Above maximum possible

    for (Student* s : students)
    {
        if (s->grades.empty()) continue;
        double avg = logic_studentAverage(*s);
        if (avg < bottomAvg)
        {
            bottomAvg = avg;
            bottom = s;
        }
    }

    return bottom;
}

std::vector<Student*> logic_filterByAverageRange(
    const std::vector<Student*>& students,
    double minGrade,
    double maxGrade)
{
    std::vector<Student*> results;
    for (Student* s : students)
    {
        double avg = logic_studentAverage(*s);
        if (avg >= minGrade && avg <= maxGrade)
            results.push_back(s);
    }
    return results;
}

std::vector<Student*> logic_findFailing(const std::vector<Student*>& students)
{
    std::vector<Student*> results;
    for (Student* s : students)
    {
        for (const SubjectGrade& sg : s->grades)
        {
            if (sg.value < 3.0)
            {
                results.push_back(s);
                break; // Only add once per student
            }
        }
    }
    return results;
}

// ═══════════════════════════════════════════════════════════════
//  Recursive Computations
// ═══════════════════════════════════════════════════════════════

/*
 * logic_recursiveAverage
 * Tail-recursive sum-accumulation over a grades array.
 *
 * Base case:  index == grades.size() → return accumulator / grades.size()
 * Recursive:  accumulate grades[index].value, advance index
 */
double logic_recursiveAverage(const std::vector<SubjectGrade>& grades,
    int index,
    double accumulator)
{
    // Base case: processed all grades
    if (grades.empty()) return 0.0;
    if (index == static_cast<int>(grades.size()))
        return accumulator / static_cast<double>(grades.size());

    // Recursive step
    return logic_recursiveAverage(grades,
        index + 1,
        accumulator + grades[index].value);
}

/*
 * logic_recursiveSumAverages
 * Recursively sum the average grade of each student in the list.
 *
 * Base case:  index == students.size() → return 0.0
 * Recursive:  avg(students[index]) + recurse(index+1)
 */
double logic_recursiveSumAverages(const std::vector<Student*>& students,
    int index)
{
    // Base case
    if (index >= static_cast<int>(students.size())) return 0.0;

    // Recursive step
    return logic_studentAverage(*students[index])
        + logic_recursiveSumAverages(students, index + 1);
}

/*
 * logic_recursiveCountFailing
 * Recursively count students with at least one grade below 3.0.
 *
 * Base case:  index == students.size() → return 0
 */
int logic_recursiveCountFailing(const std::vector<Student*>& students,
    int index)
{
    // Base case
    if (index >= static_cast<int>(students.size())) return 0;

    // Check current student
    int isFailing = 0;
    for (const SubjectGrade& sg : students[index]->grades)
    {
        if (sg.value < 3.0)
        {
            isFailing = 1;
            break;
        }
    }

    // Recursive step
    return isFailing + logic_recursiveCountFailing(students, index + 1);
}

/*
 * logic_recursiveFindMax
 * Divide-and-conquer maximum search.
 * Returns the INDEX in students[] of the student with the highest average
 * within the range [lo, hi].
 */
int logic_recursiveFindMax(const std::vector<Student*>& students,
    int lo, int hi)
{
    // Base case: single element
    if (lo == hi) return lo;

    // Recursive step: find max in each half, return the better one
    int mid = lo + (hi - lo) / 2;
    int leftM = logic_recursiveFindMax(students, lo, mid);
    int rightM = logic_recursiveFindMax(students, mid + 1, hi);

    double leftAvg = logic_studentAverage(*students[leftM]);
    double rightAvg = logic_studentAverage(*students[rightM]);

    return (leftAvg >= rightAvg) ? leftM : rightM;
}

/*
 * logic_fibonacci
 * Classic recursive Fibonacci.
 * For demo / assignment purposes only (not memoised).
 */
long long logic_fibonacci(int n)
{
    if (n <= 0) return 0;
    if (n == 1) return 1;
    return logic_fibonacci(n - 1) + logic_fibonacci(n - 2);
}

/*
 * logic_factorial
 * Recursive factorial.
 */
long long logic_factorial(int n)
{
    if (n <= 0) return 1;
    return static_cast<long long>(n) * logic_factorial(n - 1);
}

// ═══════════════════════════════════════════════════════════════
//  Averages & Statistics
// ═══════════════════════════════════════════════════════════════

double logic_studentAverage(const Student& student)
{
    if (student.grades.empty()) return 0.0;
    double sum = 0.0;
    for (const SubjectGrade& sg : student.grades)
        sum += sg.value;
    return sum / static_cast<double>(student.grades.size());
}

GroupStats logic_groupStats(const std::vector<Student*>& students,
    const std::string& groupName)
{
    GroupStats stats;
    stats.groupName = groupName;
    stats.totalStudents = static_cast<int>(students.size());
    stats.overallAverage = 0.0;
    stats.totalFailing = 0;
    stats.excellentCount = 0;

    if (students.empty()) return stats;

    // ── Per-subject accumulation ───────────────────────────────
    // Build maps indexed by Subject enum int
    struct SubjectAcc
    {
        double sum = 0.0;
        double highest = 0.0;
        double lowest = 7.0; // above max
        int    count = 0;
        int    failing = 0;
    };

    SubjectAcc acc[MAX_SUBJECTS]; // indexed by Subject enum value

    for (Student* s : students)
    {
        bool hasFailing = false;
        for (const SubjectGrade& sg : s->grades)
        {
            int idx = static_cast<int>(sg.subject);
            acc[idx].sum += sg.value;
            acc[idx].count++;
            if (sg.value > acc[idx].highest) acc[idx].highest = sg.value;
            if (sg.value < acc[idx].lowest)  acc[idx].lowest = sg.value;
            if (sg.value < 3.0)
            {
                acc[idx].failing++;
                hasFailing = true;
            }
        }
        if (hasFailing) stats.totalFailing++;

        double avg = logic_studentAverage(*s);
        if (avg >= 5.50) stats.excellentCount++;
    }

    // Build SubjectStats vector
    for (Subject subj : data_allSubjects())
    {
        int idx = static_cast<int>(subj);
        if (acc[idx].count == 0) continue;

        SubjectStats ss;
        ss.subject = subj;
        ss.average = acc[idx].sum / acc[idx].count;
        ss.highest = acc[idx].highest;
        ss.lowest = (acc[idx].lowest > 6.0) ? 0.0 : acc[idx].lowest;
        ss.count = acc[idx].count;
        ss.failing = acc[idx].failing;
        stats.bySubject.push_back(ss);
    }

    // Overall average using recursive helper
    double sumAvg = logic_recursiveSumAverages(students, 0);
    stats.overallAverage = (students.empty()) ? 0.0
        : sumAvg / static_cast<double>(students.size());

    return stats;
}

GroupStats logic_classStats(DataStore& store, const std::string& classLabel)
{
    std::vector<Student*> classStudents = data_findByClass(store, classLabel);
    return logic_groupStats(classStudents, classLabel);
}

GroupStats logic_schoolStats(DataStore& store)
{
    std::vector<Student*> all = data_getAllActive(store);
    return logic_groupStats(all, "Всички ученици");
}

std::string logic_gradeLabel(double average)
{
    if (average <= 0.0) return "No grades";
    if (average < 3.0)  return "Fail";
    if (average < 3.5)  return "Poor";
    if (average < 4.5)  return "Good";
    if (average < 5.5)  return "Very Good";
    return "Excellent";
}

int logic_gradeColorIndex(double grade)
{
    if (grade <= 0.0) return -1; // No grade
    if (grade < 3.0)  return 0;  // Red – Слаб
    if (grade < 3.5)  return 1;  // Orange – Среден (-)
    if (grade < 4.5)  return 2;  // Yellow – Добър
    if (grade < 5.5)  return 3;  // Light-green – Мн. добър
    return 4;                    // Green – Отличен
}

// ═══════════════════════════════════════════════════════════════
//  Student Management through Logic layer
// ═══════════════════════════════════════════════════════════════

LogicResult logic_validateStudent(const Student& student)
{
    if (student.firstName.empty() || student.lastName.empty())
        return LogicResult::ERR_INVALID;

    if (student.classLabel.empty())
        return LogicResult::ERR_INVALID;

    // Year group must be 1-4
    int yg = static_cast<int>(student.yearGroup);
    if (yg < 1 || yg > 4)
        return LogicResult::ERR_INVALID;

    return LogicResult::OK;
}

LogicResult logic_addStudent(DataStore& store, Student& student, int& outId)
{
    LogicResult vr = logic_validateStudent(student);
    if (vr != LogicResult::OK) return vr;

    outId = data_addStudent(store, student);
    return LogicResult::OK;
}

LogicResult logic_updateStudent(DataStore& store, const Student& student)
{
    LogicResult vr = logic_validateStudent(student);
    if (vr != LogicResult::OK) return vr;

    if (!data_updateStudent(store, student))
        return LogicResult::ERR_NOT_FOUND;

    return LogicResult::OK;
}

LogicResult logic_deleteStudent(DataStore& store, int id)
{
    if (!data_deleteStudent(store, id))
        return LogicResult::ERR_NOT_FOUND;
    return LogicResult::OK;
}

LogicResult logic_setGrade(DataStore& store, int studentId,
    Subject subject, double gradeValue,
    const std::string& note)
{
    if (!data_isValidGrade(gradeValue))
        return LogicResult::ERR_INVALID;

    if (!data_setGrade(store, studentId, subject, gradeValue, note))
        return LogicResult::ERR_NOT_FOUND;

    return LogicResult::OK;
}

LogicResult logic_removeGrade(DataStore& store, int studentId, Subject subject)
{
    if (!data_removeGrade(store, studentId, subject))
        return LogicResult::ERR_NOT_FOUND;
    return LogicResult::OK;
}

// ═══════════════════════════════════════════════════════════════
//  Persistence
// ═══════════════════════════════════════════════════════════════

LogicResult logic_save(const DataStore& store, const std::string& filepath)
{
    return data_saveToFile(store, filepath)
        ? LogicResult::OK
        : LogicResult::ERR_IO;
}

LogicResult logic_load(DataStore& store, const std::string& filepath)
{
    return data_loadFromFile(store, filepath)
        ? LogicResult::OK
        : LogicResult::ERR_IO;
}

// ═══════════════════════════════════════════════════════════════
//  Ranking
// ═══════════════════════════════════════════════════════════════

std::vector<Student*> logic_buildRanking(DataStore& store,
    const std::string& classFilter)
{
    std::vector<Student*> students;

    if (classFilter.empty())
        students = data_getAllActive(store);
    else
        students = data_findByClass(store, classFilter);

    // Sort by average descending using Quick Sort
    logic_quickSort(students, SortField::BY_AVERAGE, SortOrder::DESCENDING);

    return students;
}
