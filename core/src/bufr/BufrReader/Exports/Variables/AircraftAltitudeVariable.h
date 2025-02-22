// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "eckit/config/LocalConfiguration.h"

#include "bufr/Variable.h"


namespace bufr {
    /// \brief Exports parsed data as aircraftAltitudes using specified Mnemonics
    class AircraftAltitudeVariable final : public Variable
    {
     public:
        AircraftAltitudeVariable() = delete;
        AircraftAltitudeVariable(const std::string& exportName,
                                 const std::string& groupByField,
                                 const eckit::LocalConfiguration& conf);

        ~AircraftAltitudeVariable() final = default;

        /// \brief Get the configured mnemonics and turn them into AircraftAltitude
        /// \param map BufrDataMap that contains the parsed data for each mnemonic
        std::shared_ptr<DataObjectBase> exportData(const BufrDataMap& map) final;

        /// \brief Get a list of queries for this variable
        QueryList makeQueryList() const final;

     private:
        /// \brief makes sure the bufr data map has all the required keys.
        void checkKeys(const BufrDataMap& map);

        /// \brief get the export key string
        std::string getExportKey(const std::string& name) const;
    };
}  // namespace bufr
