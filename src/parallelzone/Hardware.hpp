#pragma once
#include "Exception.hpp"
#include <iostream>
#include <papi.h>
#include <vector>

namespace pz {

class Hardware {
    PAPI_hw_info_t* m_hwInfo = nullptr;
    std::vector<HardwareComponent*> m_hwComponent;

public:
    Hardware();

    ~Hardware();

    void get_papi_version();
};

class HWCompStorage {
public:
    size_t free_space;
    size_t global_dev_space;
};

//////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------

Hardware::Hardware() {
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if(retval != PAPI_VER_CURRENT) {
        throw ParallelZone::pz_exception("PZ: PAPI_library_init() failed",
                                         __FILE__, __LINE__);
    }

    m_hwInfo = PAPI_get_hardware_info();
    if(m_hwInfo == nullptr) {
        throw ParallelZone::pz_exception("PZ: PAPI_get_hardware_info() failed",
                                         __FILE__, __LINE__);
    }
    PAPI_dev_type_info_t* dev_type_info = m_hwInfo->dev_type_arr;

    for(int dev_type_id = 0; dev_type_id < PAPI_DEV_TYPE_ID__MAX_NUM;
        dev_type_id++) {
        if(dev_type_info[dev_type_id].id == PAPI_DEV_TYPE_ID__CPU ||
           dev_type_info[dev_type_id].id == PAPI_DEV_TYPE_ID__NVIDIA_GPU ||
           dev_type_info[dev_type_id].id == PAPI_DEV_TYPE_ID__AMD_GPU) {
            HardwareComponent* hwcomp =
              HardwareComponent(dev_type_info[dev_type_id]);
            m_hwComponent.push_back(hwcomp);
        }
    }
}

Hardware::~Hardware() {
    if(PAPI_is_initialized()) { PAPI_shutdown(); }
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------

void Hardware::get_papi_version() {
    std::cout << "PZ: Testing sysdetect component with PAPI v"
              << PAPI_VERSION_MAJOR(PAPI_VERSION) << "."
              << PAPI_VERSION_MINOR(PAPI_VERSION) << "."
              << PAPI_VERSION_REVISION(PAPI_VERSION) << std::endl;
}

//--------------------------------------------------------------------

// #include <cuda_runtime.h>
// #include <functional>
// #include <iostream>
// #include <nvml.h>
// #include <sstream>
// #include <string>
// #include <vector>

// #ifndef NVML_DEVICE_UUID_V2_BUFFER_SIZE
// #define NVML_DEVICE_UUID_V2_BUFFER_SIZE 256
// #endif

// #define ENABLE_MPI

// #ifdef ENABLE_MPI
// #include <mpi.h>
// #endif

// int main() {
// #ifdef ENABLE_MPI
//     MPI_Init(NULL, NULL);
//     int world_rank;
//     MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
//     int world_size;
//     MPI_Comm_size(MPI_COMM_WORLD, &world_size);
// #else
//     int world_rank = 0, world_size = 1;
// #endif

//     bool nvml_is_initialized = false;
//     // Cleanup execution
//     auto cleanup = [&]() {
// #ifdef ENABLE_MPI
//         MPI_Finalize();
// #endif
//         if(nvml_is_initialized) nvmlShutdown();
//     };

//     {
//         // Initialize NVML
//         if(nvmlInit() != NVML_SUCCESS) {
//             if(!world_rank) std::cout << "NVML Init Failed" << std::endl;
//             cleanup();
//             return 1;
//         }
//         nvml_is_initialized = true;

//         // Get local device count
//         unsigned int true_device_count;
//         if(nvmlDeviceGetCount(&true_device_count) != NVML_SUCCESS) {
//             if(!world_rank)
//                 std::cout << "NVML Failed to Get Device Count" << std::endl;
//             cleanup();
//             return 1;
//         }
//         int cuda_device_count;
//         cudaGetDeviceCount(&cuda_device_count);

//         if(!world_rank) {
//             std::stringstream ss;
//             ss << "RANK = " << world_rank << " NGPU = " << true_device_count
//                << " , CUDA_NGPU = " << cuda_device_count << std::endl;
//             std::cout << ss.str();
//         }

//         std::vector<int> cuda_device_counts_mpi(world_size);
// #ifdef ENABLE_MPI
//         MPI_Allgather(&cuda_device_count, sizeof(int), MPI_BYTE,
//                       cuda_device_counts_mpi.data(), sizeof(int), MPI_BYTE,
//                       MPI_COMM_WORLD);
// #else
//         cuda_device_counts_mpi[0] = cuda_device_count;
// #endif

//         if(std::any_of(cuda_device_counts_mpi.begin(),
//                        cuda_device_counts_mpi.end(),
//                        [&](auto c) { return c != cuda_device_count; })) {
//             if(!world_rank)
//                 std::cout << "This Driver Only Works If Every Rank has the "
//                              "same # devices"
//                           << std::endl;
//             cleanup();
//             return 1;
//         }

//         // Loop over local devices
//         std::vector<std::size_t> uuid_hashes(cuda_device_count);
//         for(int32_t i = 0; i < cuda_device_count; ++i) {
//             nvmlDevice_t device;

// #if 0
//     // Get device identifier via local index
//     // XXX: This doesn't respect CUDA_VISABLE_DEVICES
//     if( nvmlDeviceGetHandleByIndex(i, &device) ) {
//       if(!world_rank) std::cout << "NVML Failed To Get Device Handle for IDX
//       = " << i << std::endl; cleanup(); return 1;
//     }
// #else
//             // Get PCI identifier for this device
//             char pci_bus_str[20];
//             pci_bus_str[0] = '\0';
//             cudaDeviceGetPCIBusId(pci_bus_str, 20, i);

//             // Get NVML Device from PCI identifier
//             if(nvmlDeviceGetHandleByPciBusId(pci_bus_str, &device)) {
//                 if(!world_rank)
//                     std::cout
//                       << "NVML Failed To Get Device Handle for IDX = " << i
//                       << std::endl;
//                 cleanup();
//                 return 1;
//             }
// #endif

//             // Get UUID for device
//             char uuid_str[NVML_DEVICE_UUID_V2_BUFFER_SIZE];
//             uuid_str[0] = '\0';
//             if(nvmlDeviceGetUUID(device, uuid_str,
//                                  NVML_DEVICE_UUID_V2_BUFFER_SIZE)) {
//                 if(!world_rank)
//                     std::cout << "NVML Failed To Get UUID for IDX = " << i
//                               << std::endl;
//                 cleanup();
//                 return 1;
//             }

//             // Compute UUID HASH
//             uuid_hashes[i] = std::hash<std::string>{}(std::string(uuid_str));

//             // std::stringstream msg;
//             // msg << "RANK = " << world_rank << " DEVICE IDX = " << i << "
//             UUID
//             // = " << uuid_str << " UUID HASH = " << uuid_hashes[i] << "\n";
//             // std::cout << msg.str() << std::flush;
//         }

//         // Gather all UUIDs
//         std::vector<std::size_t> gathered_uuid_hashes(world_size *
//                                                       cuda_device_count);
// #ifdef ENABLE_MPI
//         // Gather UUID Hashes
//         MPI_Allgather(
//           uuid_hashes.data(), cuda_device_count * sizeof(std::size_t),
//           MPI_BYTE, gathered_uuid_hashes.data(), cuda_device_count *
//           sizeof(std::size_t), MPI_BYTE, MPI_COMM_WORLD);
// #else
//         std::copy(uuid_hashes.begin(), uuid_hashes.end(),
//                   gathered_uuid_hashes.begin());
// #endif

//         // Get uniq UUIDs
//         std::vector<std::size_t> uniq_uuid(gathered_uuid_hashes);
//         std::sort(uniq_uuid.begin(), uniq_uuid.end());
//         {
//             auto new_end = std::unique(uniq_uuid.begin(), uniq_uuid.end());
//             uniq_uuid.erase(new_end, uniq_uuid.end());
//         }

//         MPI_Barrier(MPI_COMM_WORLD);
//         if(!world_rank) {
//             std::cout << "Found " << uniq_uuid.size() << " Unique Devices"
//                       << std::endl;
//             for(auto uuid : uniq_uuid) {
//                 std::cout << "Device " << uuid << " Maps To:" << std::endl;
//                 auto cur_it = gathered_uuid_hashes.begin();
//                 while(cur_it != gathered_uuid_hashes.end()) {
//                     cur_it =
//                       std::find(cur_it, gathered_uuid_hashes.end(), uuid);
//                     if(cur_it != gathered_uuid_hashes.end()) {
//                         int idx =
//                           std::distance(gathered_uuid_hashes.begin(),
//                           cur_it);
//                         int mpi_rank = idx / cuda_device_count;
//                         int dev_idx  = idx % cuda_device_count;
//                         std::cout << "  RANK = " << mpi_rank
//                                   << ", CUDA_INDEX = " << dev_idx <<
//                                   std::endl;
//                         cur_it++;
//                     }
//                 }
//             }
//         }
//     }

//     cleanup();
//     return 0;
// }

} // namespace pz
