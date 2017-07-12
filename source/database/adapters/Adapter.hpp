//
// Created by svakhreev on 28.12.16.
//

#ifndef CAMERAMANAGERCORE_ADAPTER_HPP
#define CAMERAMANAGERCORE_ADAPTER_HPP

#include <vector>

namespace cameramanagercore::database::adapters
{

struct ExecuteEmpty {};
struct AddBlob { std::vector<int> blob; };

template<typename Database>
struct Adapter;

}

#endif //CAMERAMANAGERCORE_ADAPTER_HPP
