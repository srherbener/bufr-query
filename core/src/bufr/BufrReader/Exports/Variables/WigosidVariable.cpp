// (C) Copyright 2023 NOAA/NWS/NCEP/EMC

#include "WigosidVariable.h"

#include <climits>
#include <iomanip>
#include <unordered_map>

#include <ostream>
#include <time.h>
#include <vector>
#include <string>
#include <sstream>

#include "eckit/exception/Exceptions.h"

#include "bufr/DataObject.h"
#include "../../../DataObjectBuilder.h"
#include "../../../Log.h"


namespace
{
    namespace ConfKeys
    {
        const char* Wgosids = "wgosids";
        const char* Wgosisid = "wgosisid";
        const char* Wgosisnm = "wgosisnm";
        const char* Wgoslid = "wgoslid";
    }  // namespace ConfKeys
}  // namespace


namespace bufr {
    WigosidVariable::WigosidVariable(const std::string& exportName,
                                       const std::string& groupByField,
                                       const eckit::LocalConfiguration& conf) :
      Variable(exportName, groupByField, conf),
      wgosidsQuery_(conf.getString(ConfKeys::Wgosids)),
      wgosisidQuery_(conf.getString(ConfKeys::Wgosisid)),
      wgosisnmQuery_(conf.getString(ConfKeys::Wgosisnm)),
      wgoslidQuery_(conf.getString(ConfKeys::Wgoslid))
     {
        initQueryMap();
     }

    std::shared_ptr<DataObjectBase> WigosidVariable::exportData(const BufrDataMap& map)
    {
        checkKeys(map);
        static const int missingInt = DataObject<int>::missingValue();

        std::vector<std::string> wigosID;
        wigosID.reserve(map.at(getExportKey(ConfKeys::Wgosids))->size());

        auto wgosidsVar = map.at(getExportKey(ConfKeys:: Wgosids));

        // Validation
        if (!wgosidsVar->hasSamePath(map.at(getExportKey(ConfKeys::Wgosisid))) ||
            !wgosidsVar->hasSamePath(map.at(getExportKey(ConfKeys::Wgosisnm))) ||
            !wgosidsVar->hasSamePath(map.at(getExportKey(ConfKeys::Wgoslid))))
        {
            std::ostringstream errStr;
            errStr << "Wigosid variables are not all from the same path.";
            throw eckit::BadParameter(errStr.str());
        }

        for (unsigned int idx = 0; idx < map.at(getExportKey(ConfKeys::Wgosids))->size(); idx++)
        {
            auto wgosids = map.at(getExportKey(ConfKeys::Wgosids))->getAsInt(idx);
            auto wgosisid = map.at(getExportKey(ConfKeys::Wgosisid))->getAsInt(idx);
            auto wgosisnm = map.at(getExportKey(ConfKeys::Wgosisnm))->getAsInt(idx);
            auto wgoslid = map.at(getExportKey(ConfKeys::Wgoslid))->getAsString(idx);

            std::stringstream wgosAll;
            if (wgosids != missingInt &&
                wgosisid != missingInt &&
                wgosisnm != missingInt &&
                wgoslid != "")
            {
                wgosAll << wgosids <<  "-";
                wgosAll << wgosisid <<  "-";
                wgosAll << wgosisnm <<  "-";
                wgosAll << wgoslid;
                // std::cout << "WGOSID: "  <<  wgosAll.str() << std::endl;
            }
            wigosID.push_back(wgosAll.str());
        }

        const auto& wigosIds = map.at(getExportKey(ConfKeys::Wgosids));

        return DataObjectBuilder::make<std::string>(wigosID,
                                                    getExportName(),
                                                    groupByField_,
                                                    wigosIds->getDims(),
                                                    wigosIds->getPath(),
                                                    wigosIds->getDimPaths());
    }

    void WigosidVariable::checkKeys(const BufrDataMap& map)
    {
        std::vector<std::string> requiredKeys = {getExportKey(ConfKeys::Wgosids),
                                                 getExportKey(ConfKeys::Wgosisid),
                                                 getExportKey(ConfKeys::Wgosisnm),
                                                 getExportKey(ConfKeys::Wgoslid)};

        std::stringstream errStr;
        errStr << "Query ";

        bool isKeyMissing = false;
        for (const auto& key : requiredKeys)
        {
            if (map.find(key) == map.end())
            {
                isKeyMissing = true;
                errStr << key;
                break;
            }
        }

        errStr << " could not be found during export of wigosid object.";

        if (isKeyMissing)
        {
            throw eckit::BadParameter(errStr.str());
        }
    }

    QueryList WigosidVariable::makeQueryList() const
    {
        auto queries = QueryList();

        {  // Wgosids
            QueryInfo info;
            info.name = getExportKey(ConfKeys::Wgosids);
            info.query = wgosidsQuery_;
            info.groupByField = groupByField_;
            queries.push_back(info);
        }

        {  // Wgosisid
            QueryInfo info;
            info.name = getExportKey(ConfKeys::Wgosisid);
            info.query = wgosisidQuery_;
            info.groupByField = groupByField_;
            queries.push_back(info);
        }

        {  // Wgosisnm
            QueryInfo info;
            info.name = getExportKey(ConfKeys::Wgosisnm);
            info.query = wgosisnmQuery_;
            info.groupByField = groupByField_;
            queries.push_back(info);
        }

        {  // Wgoslid
            QueryInfo info;
            info.name = getExportKey(ConfKeys::Wgoslid);
            info.query = wgoslidQuery_;
            info.groupByField = groupByField_;
            queries.push_back(info);
        }

        return queries;
    }

    std::string WigosidVariable::getExportKey(const char* name) const
    {
        return getExportName() + "_" + name;
    }
}  // namespace bufr
