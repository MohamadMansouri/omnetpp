//=========================================================================
//  IDLIST.H - part of
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

#ifndef __OMNETPP_SCAVE_IDLIST_H
#define __OMNETPP_SCAVE_IDLIST_H

#include <vector>
#include <exception>
#include <stdexcept>
#include "scavedefs.h" // int64_t

namespace omnetpp {
namespace scave {

class ResultFileManager;

/**
 * Result ID -- identifies a scalar or a vector in a ResultFileManager
 */
typedef int64_t ID;

/**
 * Stores an immutable set of unique IDs. An ID identifies a result item
 * inside ResultFileManager. The primary reason that IDs and IDLists exist
 * is to make it more efficient to work with sets of results in Java.
 *
 * Immutability only refers to the set of contained IDs, the order may
 * change during the lifetime of the object (due to the fact that ordering
 * makes a number of operations more efficient).
 *
 * Contained IDs are supposed to be unique, but this is not enforced
 * for efficiency reasons.
 */
class SCAVE_API IDList
{
    private:
        friend class ResultFileManager;
        typedef std::vector<ID> V;
        V v;

        void checkIntegrity(ResultFileManager *mgr) const;
        void checkIntegrityAllScalars(ResultFileManager *mgr) const;
        void checkIntegrityAllParameters(ResultFileManager *mgr) const;
        void checkIntegrityAllVectors(ResultFileManager *mgr) const;
        void checkIntegrityAllHistograms(ResultFileManager *mgr) const;

        static void sort(/*non-*/const V& cv) {V& v = const_cast<V&>(cv); std::sort(v.begin(), v.end());}

        template <class T> void sortBy(ResultFileManager *mgr, bool ascending, T& comparator);
        template <class T> void sortScalarsBy(ResultFileManager *mgr, bool ascending, T& comparator);
        template <class T> void sortParametersBy(ResultFileManager *mgr, bool ascending, T& comparator);
        template <class T> void sortVectorsBy(ResultFileManager *mgr, bool ascending, T& comparator);
        template <class T> void sortHistogramsBy(ResultFileManager *mgr, bool ascending, T& comparator);

        void append(ID id) {v.push_back(id);} // no uniqueness check, use of discardDuplicates() recommended
        void discardDuplicates();

    public:
        IDList() {}
        IDList(ID id) {v.push_back(id);}
        IDList(const IDList& ids) {v = ids.v;}
        IDList(IDList&& ids) {v = std::move(ids.v);}
        IDList(std::vector<ID>&& ids) {v = std::move(ids);}

        // these are the only mutator methods (deliberately not accessible from Java)
        IDList& operator=(const IDList& ids) {v = ids.v; return *this;}
        IDList& operator=(IDList&& ids) {v = std::move(ids.v); return *this;}

        bool equals(IDList& other);
        int64_t hashCode64() const;

        // element access
        bool isEmpty() const  {return v.empty();}
        int size() const  {return (int)v.size();}
        ID get(int i) const {return v.at(i);} // at() includes bounds check
        int indexOf(ID x) const;
        const std::vector<ID>& asVector() const {return v;}

        // support for range-based for loops
        V::const_iterator begin() const {return v.begin();}
        V::const_iterator end() const {return v.end();}

        // set operations
        IDList unionWith(IDList& ids) const;
        IDList subtract(IDList& ids) const;
        IDList intersect(IDList& ids) const;
        bool isSubsetOf(IDList& ids);

        // filtering
        IDList getRange(int startIndex, int endIndex) const;
        IDList getSubsetByIndices(int *array, int n) const;

        // query by item types
        int getItemTypes() const;  // SCALAR, VECTOR or their binary OR
        bool areAllScalars() const;
        bool areAllParameters() const;
        bool areAllVectors() const;
        bool areAllStatistics() const;
        bool areAllHistograms() const;
        IDList filterByTypes(int typeMask) const;
        int countByTypes(int typeMask) const;

        // sorting
        // TODO: there's a duplication between vector and histogram sorting due to not having a proper superclass with the statistics inside
        void sort() {std::sort(v.begin(), v.end());}  // sort numerically; getUniqueFileRuns() etc are faster on sorted IDLists
        void sortByFileAndRun(ResultFileManager *mgr, bool ascending);
        void sortByRunAndFile(ResultFileManager *mgr, bool ascending);
        void sortByDirectory(ResultFileManager *mgr, bool ascending);
        void sortByFileName(ResultFileManager *mgr, bool ascending);
        void sortByRun(ResultFileManager *mgr, bool ascending);
        void sortByModule(ResultFileManager *mgr, bool ascending);
        void sortByName(ResultFileManager *mgr, bool ascending);
        void sortScalarsByValue(ResultFileManager *mgr, bool ascending);
        void sortParametersByValue(ResultFileManager *mgr, bool ascending);
        void sortVectorsByVectorId(ResultFileManager *mgr, bool ascending);
        void sortVectorsByLength(ResultFileManager *mgr, bool ascending);
        void sortVectorsByMean(ResultFileManager *mgr, bool ascending);
        void sortVectorsByStdDev(ResultFileManager *mgr, bool ascending);
        void sortVectorsByMin(ResultFileManager *mgr, bool ascending);
        void sortVectorsByMax(ResultFileManager *mgr, bool ascending);
        void sortVectorsByVariance(ResultFileManager *mgr, bool ascending);
        void sortVectorsByStartTime(ResultFileManager *mgr, bool ascending);
        void sortVectorsByEndTime(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByLength(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByMean(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByStdDev(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByMin(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByMax(ResultFileManager *mgr, bool ascending);
        void sortHistogramsByVariance(ResultFileManager *mgr, bool ascending);
        void sortByRunAttribute(ResultFileManager *mgr, const char* runAttr, bool ascending);
        void reverse();
};

} // namespace scave
}  // namespace omnetpp


#endif


