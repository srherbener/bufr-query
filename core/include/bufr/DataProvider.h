// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <functional>
#include <set>
#include <string>
#include <vector>
#include <math.h>
#include <memory>
#include <gsl/gsl-lite.hpp>
#include <unordered_map>

#include "bufr_interface.h"
#include "QuerySet.h"
#include "SubsetVariant.h"


namespace bufr {
    typedef unsigned int FortranIdx;

    /// \brief Enum for elements stored in the BUFR lib TYP variable.
    enum class Typ
    {
        Subset,
        DelayedRep,
        FixedRep,
        DelayedRepStacked,
        DelayedBinary,
        Sequence,
        Repeat,
        StackedRepeat,
        Number,
        Character
    };

    /// \brief Translation map for strings stored in the BUFR lib TYP variable. Maps
    ///        typ strings to Typ enum values.
    const std::unordered_map<std::string, Typ> TypMap =
        {{"SUB",            Typ::Subset},
         {"DRP",        Typ::DelayedRep},
         {"REP",          Typ::FixedRep},
         {"DRS", Typ::DelayedRepStacked},
         {"DRB",     Typ::DelayedBinary},
         {"SEQ",          Typ::Sequence},
         {"RPC",            Typ::Repeat},
         {"RPS",     Typ::StackedRepeat},
         {"NUM",            Typ::Number},
         {"CHR",         Typ::Character}};


    struct TypeInfo
    {
        int scale = 0;
        int reference = 0;
        int bits = 0;
        std::string unit;
        std::string description;

        bool isString() const { return unit == "CCITT IA5"; }
        bool isSigned() const
        {
            // To better support Fortran clients for the generated ObsGroups we will assume all
            // fields are signed. Otherwise this code would be reference < 0.
            return true;
        }
        bool isInteger() const { return scale <= 0; }
        bool is64Bit() const
        {
            if (bits == 0) return false;

            bool is64Bit;
            if (isInteger() && !isSigned())
            {
                is64Bit = (log2((pow(2, bits) - 1) / pow(10, scale) + reference) > 32);
            }
            else if (isInteger() && isSigned())
            {
                is64Bit = (log2(fmax(-1 * reference,
                    (pow(2, bits - 1) - 1) / pow(10, scale) + reference) * 2) + 1 > 32);
            }
            else
            {
                is64Bit = false;
            }

            return is64Bit;
        }

        bool isLongString() const
        {
            return isString() && bits > 64;
        }
    };

    /// \brief Table to hold subset table meta data
    struct TableData
    {
        // Table data;
        std::string subset;
        std::vector<int> isc;
        std::vector<int> link;
        std::vector<int> itp;
        std::vector<int> jmpb;
        std::vector<int> irf;
        std::vector<Typ> typ;
        std::vector<std::string> tag;
        int varientNumber;
    };

    class DataProvider;
    typedef std::shared_ptr<DataProvider> DataProviderType;

    /// \brief Responsible for exposing the data found in a BUFR file.
    class DataProvider
    {
     public:
        DataProvider() = delete;

        explicit DataProvider(const std::string filePath) :
            filePath_(filePath)
        {
        }

        virtual ~DataProvider() = default;

        /// \brief Runs through the contents of the BUFR file. Calls the functions given as
        ///        its running.
        /// \param processSubset The function to call to process a subset.
        /// \param processMsg (Optional) Function to call when finish processing a message.
        /// \param continueProcessing (Optional) Function to call to figure out if we should keep
        ///                           running or not.
        void run(const QuerySet& querySet,
                 const std::function<void()> processSubset,
                 const std::function<void()> processMsg = [](){},
                 const std::function<bool()> continueProcessing = [](){ return true; },
                 size_t offset = 0);

        /// \brief Open the BUFR file with NCEPLIB-bufr
        virtual void open() = 0;

        /// \brief Close the currently open BUFR file.
        virtual void close() = 0;

        /// \brief Rewind the current BUFR file (start over from the beginning).
        void rewind()
        {
            close();
            open();
        }

        size_t numMessages(const QuerySet& querySet);

        /// \brief Is the BUFR file open
        bool isFileOpen() { return isOpen_; }

        /// \brief Tells the Fortran BUFR interface to delete its temporary data structures that are
        /// are needed to support this class instanc.
        inline void deleteData() { delete_table_data_f(); }

        /// \brief Get the current active subset variant.
        SubsetVariant getSubsetVariant() const
        {
            return SubsetVariant(subset_, variantId(), hasVariants());
        }

        /// \brief Get the filepath for the currently open BUFR file.
        std::string getFilepath() const { return filePath_; }

        /// \brief Get the initial (start) BUFR table node for that
        ///        that corresponds to the data.
        inline FortranIdx getInode() const { return inode_; }

        /// \brief Get the number of data values in the current BUFR subset.
        inline FortranIdx getNVal() const { return nval_; }

        /// \brief Given the current offset in the data, this function will give you the ID of the
        ///        corresponding BUFR table node.
        /// \param idx A data offset value.
        inline FortranIdx getInv(FortranIdx idx) const { return inv_[idx - 1]; }

        /// \brief Get the value of the data element at the given data index.
        /// \param The index of the data object for which you want a value.
        inline double getVal(FortranIdx idx) const { return val_[idx - 1]; }

        /// \brief Get the value of the data element at the given data index.
        /// \param The index of the data object for which you want a value.
        inline gsl::span<const double> getVals() const { return val_; }

        std::string getLongStr(const std::string& longStrId) const;

        /// \brief Get the TypeInfo object for the table node at the given idx.
        /// \param idx BUFR table node index
        TypeInfo getTypeInfo(FortranIdx idx) const;

        /// \brief Given the initial BUFR table node idx (see getInode), this function returns
        ///        the node idx for the last BUFR table element for the subset. Valid while
        ///        executing "run".
        /// \param idx BUFR table node index
        inline FortranIdx getIsc(FortranIdx idx) const { return getTableData()->isc[idx - 1]; }

        /// \brief Given a BUFR table node index, this function returns the next logical node in the
        ///        tree... Valid while executing "run".
        /// \param idx BUFR table node index
        inline FortranIdx getLink(FortranIdx idx) const { return getTableData()->link[idx - 1]; }

        /// \brief Given a BUFR table node index, this function can give you some type information
        ///        for example a value of 3 is used for strings. Valid while executing "run".
        /// \param idx BUFR table node index
        inline FortranIdx getItp(FortranIdx idx) const { return getTableData()->itp[idx - 1]; }

        /// \brief Given a BUFR table node index, gives you the node idx for the node that is the
        ///        the next one up in the hierarchy. WARNING: will return 0 for any node at the end
        ///        of any sequence. Valid while executing "run".
        /// \param idx BUFR table node index
        inline FortranIdx getJmpb(FortranIdx idx) const { return getTableData()->jmpb[idx - 1]; }

        /// \brief Given a BUFR table node index, gives you a way to discover the number of repeats
        ///        in a fixed replication sequence. Valid while executing "run".
        inline FortranIdx getIrf(FortranIdx idx) const { return getTableData()->irf[idx - 1]; }

        /// \brief Given a BUFR table node index, returns the type (see the Typ enum and maps above)
        ///        Valid while executing "run".
        /// \param idx BUFR table node index
        inline Typ getTyp(FortranIdx idx) const { return getTableData()->typ[idx - 1]; }

        /// \brief Given a BUFR table node index, returns the tag (name as a human readable string)
        ///        Valid while executing "run".
        /// \param idx BUFR table node index
        inline std::string getTag(FortranIdx idx) const { return getTableData()->tag[idx - 1]; }

        /// \brief Gets the variant number for the currently loaded subset.
        virtual size_t variantId() const = 0;

        /// \brief Returns true if more than one variant have been detected for the currently
        ///        loaded subset.
        virtual bool hasVariants() const = 0;

        /// \brief Initialize the table cache in order to capture all the subset information.
        virtual void initAllTableData() {}

     protected:
        static const int FileUnit = 12;

        const std::string filePath_;
        std::string subset_;
        bool isOpen_ = false;

        // BUFR table meta data elements
        int inode_;
        int nval_;
        int bufrLoc_;

        // BUFR table element arrays
        gsl::span<const double> val_;
        gsl::span<const int> inv_;

        /// \brief Update the table data for the currently loaded subset.
        /// \param subset The subset string.
        virtual void updateTableData(const std::string& subset) = 0;

        /// \brief Read the data from the BUFR interface for the current subset and reset the
        /// internal data structures.
        ////// \param bufrLoc The Fortran idx for the subset we need to read.
        void updateData(int bufrLoc);

     private:
        /// \brief Get the currently valid subset table data
        virtual std::shared_ptr<TableData> getTableData() const = 0;
    };
}  // namespace bufr
