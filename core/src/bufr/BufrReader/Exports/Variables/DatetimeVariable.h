// (C) Copyright 2020 NOAA/NWS/NCEP/EMC

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/Variable.h"


namespace bufr {
    /// \brief Exports parsed data as datetimes using speciefied Mnemonics
    class DatetimeVariable final : public Variable
    {
     public:
        DatetimeVariable() = delete;
        DatetimeVariable(const std::string& exportName,
                         const std::string& groupByField,
                         const eckit::LocalConfiguration& conf);
        ~DatetimeVariable() final = default;

        /// \brief Get the configured mnemonics and turn them into datetime strings
        /// \param map BufrDataMap that contains the parsed data for each mnemonic
        std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& map) final;

        /// \brief Get a list of queries for this variable
        QueryList makeQueryList() const final;

     private:
        /// \brief Query for year
        const std::string yearQuery_;

        /// \brief Query for month
        const std::string monthQuery_;

        /// \brief Query for day
        const std::string dayQuery_;

        /// \brief Query for hour
        const std::string hourQuery_;

        /// \brief Query for minute
        const std::string minuteQuery_;

        /// \brief Query for second (optional)
        std::string secondQuery_;

        /// \brief Hours to offset from UTC (optional)
        int hoursFromUtc_;

        /// \brief makes sure the bufr data map has all the required keys.
        void checkKeys(const BufrDataMap& map);

        /// \brief get the export key string
        std::string getExportKey(const char* name) const;
    };
}  // namespace bufr
