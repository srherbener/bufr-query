// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include "DataProvider.h"

#include <gsl/gsl-lite.hpp>


namespace bufr {
    class QuerySet;

    /// \brief This data provider is used to read standard NCEP files. The complete listing
    ///        of subset table data can be found in the first message of the file, so we only
    ///        need to store this data one time.
    class NcepDataProvider : public DataProvider
    {
     public:
        explicit NcepDataProvider(const std::string& filePath_);

        /// \brief Open the BUFR file with NCEPLIB-bufr
        void open() final;

        /// \brief Close the currently open BUFR file.
        void close() final;

        /// \brief Gets the variant number for the currently loaded subset.
        size_t variantId() const final;

        /// \brief Returns true if more than one variant have been detected for the currently
        ///        loaded subset.
        bool hasVariants() const final;

     private:
        /// \brief Data for subset table data
        std::shared_ptr<TableData> currentTableData_ = nullptr;

        /// \brief Update the table data for the currently loaded subset.
        /// \param subset The subset string.
        void updateTableData(const std::string& subset) final;

        /// \brief Get the currently valid subset table data
        inline std::shared_ptr<TableData> getTableData() const final { return currentTableData_; }
    };
}  // namespace bufr
