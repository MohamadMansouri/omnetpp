//=========================================================================
//  IDLIST.CC - part of
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  Author: Andras Varga, Tamas Borbely
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

// turn off 'Deprecated std::copy()' warning (MSVC80)
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <cstdlib>
#include <algorithm>
#include <functional>
#include "common/stringutil.h"
#include "idlist.h"
#include "resultfilemanager.h"
#include "scaveutils.h"

#ifdef THREADED
#include "common/rwlock.h"
#define READER_MUTEX    Mutex __reader_mutex_(mgr->getReadLock());
#else
#define READER_MUTEX
#endif

using namespace omnetpp::common;

namespace omnetpp {
namespace scave {

void IDList::discardDuplicates()
{
    sort(v);
    auto last = std::unique(v.begin(), v.end());
    v.erase(last, v.end());
}

int IDList::indexOf(ID x) const
{
    auto it = std::find(v.begin(), v.end(), x);
    if (it != v.end())
        return (int)(it - v.begin());
    else
        return -1;
}

IDList IDList::unionWith(IDList& ids) const
{
    // sort both vectors so that we can apply set_union
    sort(v);
    sort(ids.v);

    // allocate a new vector, and merge the two vectors into it
    IDList result;
    V& v2 = result.v;
    v2.resize(v.size() + ids.v.size());
    auto v2end = std::set_union(v.begin(), v.end(), ids.v.begin(), ids.v.end(), v2.begin());
    v2.resize(v2end - v2.begin());
    return result;
}

IDList IDList::subtract(IDList& ids) const
{
    // sort both vectors so that we can apply set_difference
    sort(v);
    sort(ids.v);

    // allocate a new vector, and compute difference into it
    IDList result;
    V& v2 = result.v;
    v2.resize(v.size());
    auto v2end = std::set_difference(v.begin(), v.end(), ids.v.begin(), ids.v.end(), v2.begin());
    v2.resize(v2end - v2.begin());
    return result;
}

IDList IDList::intersect(IDList& ids) const
{
    // sort both vectors so that we can apply set_intersect
    sort(v);
    sort(ids.v);

    // allocate a new vector, and compute intersection into it
    IDList result;
    V& v2 = result.v;
    v2.resize(v.size());
    auto v2end = std::set_intersection(v.begin(), v.end(), ids.v.begin(), ids.v.end(), v2.begin());
    v2.resize(v2end - v2.begin());
    return result;
}

bool IDList::isSubsetOf(IDList& ids)
{
    // sort both vectors so that we can apply std::includes()
    sort(v);
    sort(ids.v);

    return std::includes(ids.v.begin(), ids.v.end(), v.begin(), v.end()); // "ids includes this"
}

IDList IDList::getRange(int startIndex, int endIndex) const
{
    if (startIndex < 0 || endIndex > v.size() || startIndex > endIndex)
        throw opp_runtime_error("Bad start or end index");
    IDList newList;
    newList.v.resize(endIndex-startIndex);
    for (int i = startIndex; i < endIndex; i++)
        newList.v[i-startIndex] = v[i];
    return newList;
}

IDList IDList::getSubsetByIndices(int *indices, int n) const
{
    IDList newList;
    for (int i = 0; i < n; i++)
        newList.v.push_back(v.at(indices[i]));
    return newList;
}

void IDList::checkIntegrity(ResultFileManager *mgr) const
{
    for (ID id : v)
        mgr->getItem(id);  // this will throw exception if id is not valid
}

void IDList::checkIntegrityAllScalars(ResultFileManager *mgr) const
{
    checkIntegrity(mgr);
    if (!areAllScalars())
        throw opp_runtime_error("These items are not all scalars");
}

void IDList::checkIntegrityAllParameters(ResultFileManager *mgr) const
{
    checkIntegrity(mgr);
    if (!areAllParameters())
        throw opp_runtime_error("These items are not all parameters");
}

void IDList::checkIntegrityAllVectors(ResultFileManager *mgr) const
{
    checkIntegrity(mgr);
    if (!areAllVectors())
        throw opp_runtime_error("These items are not all vectors");
}

void IDList::checkIntegrityAllHistograms(ResultFileManager *mgr) const
{
    checkIntegrity(mgr);
    if (!areAllHistograms())
        throw opp_runtime_error("These items are not all histograms");
}

class CmpBase : public std::binary_function<ID, ID, bool> {
    protected:
       ResultFileManager *mgr;
       bool less(const std::string& a, const std::string& b)
          {return strdictcmp(a.c_str(), b.c_str()) < 0;}
       const ResultItem& uncheckedGetItem(ID id) const { return mgr->uncheckedGetItem(id); }
       const ScalarResult& uncheckedGetScalar(ID id) const { return mgr->uncheckedGetScalar(id); }
       const ParameterResult& uncheckedGetParameter(ID id) const { return mgr->uncheckedGetParameter(id); }
       const VectorResult& uncheckedGetVector(ID id) const { return mgr->uncheckedGetVector(id); }
       const StatisticsResult& uncheckedGetStatistics(ID id) const { return mgr->uncheckedGetStatistics(id); }
       const HistogramResult& uncheckedGetHistogram(ID id) const { return mgr->uncheckedGetHistogram(id); }
    public:
        CmpBase(ResultFileManager *m) {mgr = m;}
};

class FileAndRunLess : public CmpBase {
    public:
        FileAndRunLess(ResultFileManager *m) : CmpBase(m) {}
        bool operator()(ID a, ID b) { // implements operator<
            const FileRun *da = uncheckedGetItem(a).getFileRun();
            const FileRun *db = uncheckedGetItem(b).getFileRun();
            if (da==db)
                return false;
            else if (da->getFile()==db->getFile())
                return da->getRun()->getRunName() < db->getRun()->getRunName();
            else
                return da->getFile()->getFilePath() < db->getFile()->getFilePath();
        }
};

class RunAndFileLess : public CmpBase {
    public:
        RunAndFileLess(ResultFileManager *m) : CmpBase(m) {}
        bool operator()(ID a, ID b) { // implements operator<
            const FileRun *da = uncheckedGetItem(a).getFileRun();
            const FileRun *db = uncheckedGetItem(b).getFileRun();
            if (da==db)
                return false;
            else if (da->getRun()==db->getRun())
                return da->getFile()->getFilePath() < db->getFile()->getFilePath();
            else
                return da->getRun()->getRunName() < db->getRun()->getRunName();
        }
};

class RunAttributeLess : public CmpBase {
    private:
        const char* attrName;
    public:
        RunAttributeLess(ResultFileManager* m, const char* attrName)
            : CmpBase(m), attrName(attrName) {}
        bool operator()(ID a, ID b) {
            const std::string& aValue = uncheckedGetItem(a).getRun()->getAttribute(attrName);
            const std::string& bValue = uncheckedGetItem(b).getRun()->getAttribute(attrName);
            return less(aValue, bValue);
        }
};

#define CMP(clazz,method) class clazz : public CmpBase { \
    public: \
        clazz(ResultFileManager *m) : CmpBase(m) {} \
        bool operator()(const ID a, const ID b) {return method;} \
    };

CMP(DirectoryLess, less(uncheckedGetItem(a).getFile()->getDirectory(), uncheckedGetItem(b).getFile()->getDirectory()))
CMP(FileNameLess, less(uncheckedGetItem(a).getFile()->getFileName(), uncheckedGetItem(b).getFile()->getFileName()))
CMP(RunLess, less(uncheckedGetItem(a).getRun()->getRunName(), uncheckedGetItem(b).getRun()->getRunName()))
CMP(ModuleLess, less(uncheckedGetItem(a).getModuleName(), uncheckedGetItem(b).getModuleName()))
CMP(NameLess, less(uncheckedGetItem(a).getName(), uncheckedGetItem(b).getName()))
CMP(ScalarValueLess, uncheckedGetScalar(a).getValue() < uncheckedGetScalar(b).getValue())
CMP(ParameterValueLess, less(uncheckedGetParameter(a).getValue(), uncheckedGetParameter(b).getValue()))

CMP(VectorIdLess, uncheckedGetVector(a).getVectorId() < uncheckedGetVector(b).getVectorId())
CMP(VectorCountLess, uncheckedGetVector(a).getStatistics().getCount() < uncheckedGetVector(b).getStatistics().getCount())
CMP(VectorMeanLess, uncheckedGetVector(a).getStatistics().getMean() < uncheckedGetVector(b).getStatistics().getMean())
CMP(VectorStddevLess, uncheckedGetVector(a).getStatistics().getStddev() < uncheckedGetVector(b).getStatistics().getStddev())
CMP(VectorMinLess, uncheckedGetVector(a).getStatistics().getMin() < uncheckedGetVector(b).getStatistics().getMin())
CMP(VectorMaxLess, uncheckedGetVector(a).getStatistics().getMax() < uncheckedGetVector(b).getStatistics().getMax())
CMP(VectorVarianceLess, uncheckedGetVector(a).getStatistics().getVariance() < uncheckedGetVector(b).getStatistics().getVariance())
CMP(StartTimeLess, uncheckedGetVector(a).getStartTime() < uncheckedGetVector(b).getStartTime())
CMP(EndTimeLess, uncheckedGetVector(a).getEndTime() < uncheckedGetVector(b).getEndTime())

CMP(StatisticsCountLess, uncheckedGetStatistics(a).getStatistics().getCount() < uncheckedGetStatistics(b).getStatistics().getCount())
CMP(StatisticsMeanLess, uncheckedGetStatistics(a).getStatistics().getMean() < uncheckedGetStatistics(b).getStatistics().getMean())
CMP(StatisticsStddevLess, uncheckedGetStatistics(a).getStatistics().getStddev() < uncheckedGetStatistics(b).getStatistics().getStddev())
CMP(StatisticsMinLess, uncheckedGetStatistics(a).getStatistics().getMin() < uncheckedGetStatistics(b).getStatistics().getMin())
CMP(StatisticsMaxLess, uncheckedGetStatistics(a).getStatistics().getMax() < uncheckedGetStatistics(b).getStatistics().getMax())
CMP(StatisticsVarianceLess, uncheckedGetStatistics(a).getStatistics().getVariance() < uncheckedGetStatistics(b).getStatistics().getVariance())

CMP(HistogramCountLess, uncheckedGetHistogram(a).getStatistics().getCount() < uncheckedGetHistogram(b).getStatistics().getCount())
CMP(HistogramMeanLess, uncheckedGetHistogram(a).getStatistics().getMean() < uncheckedGetHistogram(b).getStatistics().getMean())
CMP(HistogramStddevLess, uncheckedGetHistogram(a).getStatistics().getStddev() < uncheckedGetHistogram(b).getStatistics().getStddev())
CMP(HistogramMinLess, uncheckedGetHistogram(a).getStatistics().getMin() < uncheckedGetHistogram(b).getStatistics().getMin())
CMP(HistogramMaxLess, uncheckedGetHistogram(a).getStatistics().getMax() < uncheckedGetHistogram(b).getStatistics().getMax())
CMP(HistogramVarianceLess, uncheckedGetHistogram(a).getStatistics().getVariance() < uncheckedGetHistogram(b).getStatistics().getVariance())

template<class T>
void IDList::sortBy(ResultFileManager *mgr, bool ascending, T& comparator)
{
    READER_MUTEX
        checkIntegrity(mgr);
    // optimization: maybe it's sorted the other way round, so we reverse it to speed up sorting
    if (v.size() >= 2 && comparator(v.at(0), v.at(v.size()-1)) != ascending)
        reverse();
    if (ascending)
        std::sort(v.begin(), v.end(), comparator);
    else
        std::sort(v.begin(), v.end(), flipArgs(comparator));
}

template<class T>
void IDList::sortScalarsBy(ResultFileManager *mgr, bool ascending, T& comparator)
{
    READER_MUTEX
        checkIntegrityAllScalars(mgr);
    // optimization: maybe it's sorted the other way round, so we reverse it to speed up sorting
    if (v.size() >= 2 && comparator(v.at(0), v.at(v.size()-1)) != ascending)
        reverse();
    if (ascending)
        std::sort(v.begin(), v.end(), comparator);
    else
        std::sort(v.begin(), v.end(), flipArgs(comparator));
}

template<class T>
void IDList::sortParametersBy(ResultFileManager *mgr, bool ascending, T& comparator)
{
    READER_MUTEX
        checkIntegrityAllParameters(mgr);
    // optimization: maybe it's sorted the other way round, so we reverse it to speed up sorting
    if (v.size() >= 2 && comparator(v.at(0), v.at(v.size()-1)) != ascending)
        reverse();
    if (ascending)
        std::sort(v.begin(), v.end(), comparator);
    else
        std::sort(v.begin(), v.end(), flipArgs(comparator));
}

template<class T>
void IDList::sortVectorsBy(ResultFileManager *mgr, bool ascending, T& comparator)
{
    READER_MUTEX
        checkIntegrityAllVectors(mgr);
    // optimization: maybe it's sorted the other way round, so we reverse it to speed up sorting
    if (v.size() >= 2 && comparator(v.at(0), v.at(v.size()-1)) != ascending)
        reverse();
    if (ascending)
        std::sort(v.begin(), v.end(), comparator);
    else
        std::sort(v.begin(), v.end(), flipArgs(comparator));
}

template<class T>
void IDList::sortHistogramsBy(ResultFileManager *mgr, bool ascending, T& comparator)
{
    READER_MUTEX
        checkIntegrityAllHistograms(mgr);
    // optimization: maybe it's sorted the other way round, so we reverse it to speed up sorting
    if (v.size() >= 2 && comparator(v.at(0), v.at(v.size()-1)) != ascending)
        reverse();
    if (ascending)
        std::sort(v.begin(), v.end(), comparator);
    else
        std::sort(v.begin(), v.end(), flipArgs(comparator));
}

void IDList::sortByFileAndRun(ResultFileManager *mgr, bool ascending)
{
    FileAndRunLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByRunAndFile(ResultFileManager *mgr, bool ascending)
{
    RunAndFileLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByDirectory(ResultFileManager *mgr, bool ascending)
{
    DirectoryLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByFileName(ResultFileManager *mgr, bool ascending)
{
    FileNameLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByRun(ResultFileManager *mgr, bool ascending)
{
    RunLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByModule(ResultFileManager *mgr, bool ascending)
{
    ModuleLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortByName(ResultFileManager *mgr, bool ascending)
{
    NameLess compare(mgr);
    sortBy(mgr, ascending, compare);
}

void IDList::sortScalarsByValue(ResultFileManager *mgr, bool ascending)
{
    ScalarValueLess compare(mgr);
    sortScalarsBy(mgr, ascending, compare);
}

void IDList::sortParametersByValue(ResultFileManager *mgr, bool ascending)
{
    ParameterValueLess compare(mgr);
    sortParametersBy(mgr, ascending, compare);
}

void IDList::sortVectorsByVectorId(ResultFileManager *mgr, bool ascending)
{
    VectorIdLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByLength(ResultFileManager *mgr, bool ascending)
{
    VectorCountLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByMean(ResultFileManager *mgr, bool ascending)
{
    VectorMeanLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByStdDev(ResultFileManager *mgr, bool ascending)
{
    VectorStddevLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByMin(ResultFileManager *mgr, bool ascending)
{
    VectorMinLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByMax(ResultFileManager *mgr, bool ascending)
{
    VectorMaxLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByVariance(ResultFileManager *mgr, bool ascending)
{
    VectorVarianceLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByStartTime(ResultFileManager *mgr, bool ascending)
{
    StartTimeLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortVectorsByEndTime(ResultFileManager *mgr, bool ascending)
{
    EndTimeLess compare(mgr);
    sortVectorsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByLength(ResultFileManager *mgr, bool ascending)
{
    HistogramCountLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByMean(ResultFileManager *mgr, bool ascending)
{
    HistogramMeanLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByStdDev(ResultFileManager *mgr, bool ascending)
{
    HistogramStddevLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByMin(ResultFileManager *mgr, bool ascending)
{
    HistogramMinLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByMax(ResultFileManager *mgr, bool ascending)
{
    HistogramMaxLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortHistogramsByVariance(ResultFileManager *mgr, bool ascending)
{
    HistogramVarianceLess compare(mgr);
    sortHistogramsBy(mgr, ascending, compare);
}

void IDList::sortByRunAttribute(ResultFileManager *mgr, const char *runAttribute, bool ascending)
{
    RunAttributeLess compare(mgr, runAttribute);
    sortBy(mgr, ascending, compare);
}

void IDList::reverse()
{
    std::reverse(v.begin(), v.end());
}

int IDList::getItemTypes() const
{
    int types = 0;
    for (ID id : v)
        types |= ResultFileManager::_type(id);
    return types;
}

bool IDList::areAllScalars() const
{
    int types = getItemTypes();
    return !types || types == ResultFileManager::SCALAR;
}

bool IDList::areAllParameters() const
{
    int types = getItemTypes();
    return !types || types == ResultFileManager::PARAMETER;
}

bool IDList::areAllVectors() const
{
    int types = getItemTypes();
    return !types || types == ResultFileManager::VECTOR;
}

bool IDList::areAllStatistics() const
{
    int types = getItemTypes();
    return !types || types == ResultFileManager::STATISTICS;
}

bool IDList::areAllHistograms() const
{
    int types = getItemTypes();
    return !types || types == ResultFileManager::HISTOGRAM;
}

IDList IDList::filterByTypes(int typeMask) const
{
    IDList result;
    for (ID id : v)
        if ((ResultFileManager::_type(id) & typeMask) != 0)
            result.v.push_back(id);

    return result;
}

int IDList::countByTypes(int typeMask) const
{
    int count = 0;
    for (ID id : v)
        if ((ResultFileManager::_type(id) & typeMask) != 0)
            count++;
    return count;
}

bool IDList::equals(IDList& other)
{
    if (v.size() != other.v.size())
        return false;
    if (v == other.v)
        return true;
    std::sort(v.begin(), v.end());
    std::sort(other.v.begin(), other.v.end());
    return v == other.v;
}

int64_t IDList::hashCode64() const
{
    int64_t x = size();
    for (ID id : v)
        x = x*31 + id;
    return x;
}

}  // namespace scave
}  // namespace omnetpp

