#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>
#include <bits/stdc++.h>

using namespace std;

enum RequestType {
    CREATE = 1,
    READ = 2,
    UPDATE = 3,
    DELETE = 4
};

enum MessageType {
    CLIENT_REQUEST,
    CLIENT_RESPONSE,
    NODE_REQUEST,
    NODE_RESPONSE
};

enum TwoPC {
    PREPARE = 1,
    COMMIT = 2,
    ROLLBACK = 3
};

namespace mpi_manager {
    inline void send_string(const string& str, const int dest, const int tag, MPI_Comm comm) {
        const int str_size = static_cast<int>(str.size());
        // Use well-separated tags to avoid conflicts
        const int size_tag = tag * 2;
        const int data_tag = tag * 2 + 1;

        MPI_Send(&str_size, 1, MPI_INT, dest, size_tag, comm);
        if (str_size > 0) {
            MPI_Send(str.c_str(), str_size, MPI_CHAR, dest, data_tag, comm);
        }
    }

    inline string receive_string(const int source, const int tag, MPI_Comm comm) {
        int str_size;
        const int size_tag = tag * 2;
        const int data_tag = tag * 2 + 1;

        MPI_Recv(&str_size, 1, MPI_INT, source, size_tag, comm, MPI_STATUS_IGNORE);

        if (str_size > 0) {
            vector<char> buffer(str_size);
            MPI_Recv(buffer.data(), str_size, MPI_CHAR, source, data_tag, comm, MPI_STATUS_IGNORE);
            return string(buffer.begin(), buffer.end());
        }
        return "";
    }

    inline void send_int(const int& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    inline void send_bool(const bool& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_CXX_BOOL, dest, tag, comm);
    }

    inline void send_enum(const int& value, const int dest, const int tag, MPI_Comm comm) {
        MPI_Send(&value, 1, MPI_INT, dest, tag, comm);
    }

    inline int receive_int(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    inline bool receive_bool(const int source, const int tag, MPI_Comm comm) {
        bool value = false;
        MPI_Recv(&value, 1, MPI_CXX_BOOL, source, tag, comm, MPI_STATUS_IGNORE);
        return value;
    }

    inline RequestType receive_request_type(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return static_cast<RequestType>(value);
    }

    inline TwoPC receive_phase_type(const int source, const int tag, MPI_Comm comm) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, source, tag, comm, MPI_STATUS_IGNORE);
        return static_cast<TwoPC>(value);
    }
}

struct NodeResponse {
    bool success{};
    string value;

    static void send_node_response(const NodeResponse& response, const int dest, const int tag, MPI_Comm comm) {
        mpi_manager::send_bool(response.success, dest, tag, comm);
        mpi_manager::send_string(response.value, dest, tag + 1, comm);
    }

    static NodeResponse receive_node_response(const int source, const int tag, MPI_Comm comm) {
        NodeResponse response;
        response.success = mpi_manager::receive_bool(source, tag, comm);
        response.value = mpi_manager::receive_string(source, tag + 1, comm);
        return response;
    }
};

struct ClientRequest {
    int client_rank{};
    RequestType type{};
    int key{};
    string value;

    static void send_client_request(const ClientRequest& request, const int dest, const int tag, MPI_Comm comm) {
        mpi_manager::send_int(request.client_rank, dest, tag, comm);
        mpi_manager::send_enum(request.type, dest, tag + 1, comm);
        mpi_manager::send_int(request.key, dest, tag + 2, comm);
        mpi_manager::send_string(request.value, dest, tag + 3, comm);
    }

    static void send_client_response(const NodeResponse& response, const int dest, const int tag, MPI_Comm comm) {
        mpi_manager::send_bool(response.success, dest, tag, comm);
        mpi_manager::send_string(response.value, dest, tag + 1, comm);
    }

    static ClientRequest receive_client_request(const int source, const int tag, MPI_Comm comm) {
        ClientRequest request;
        request.client_rank = mpi_manager::receive_int(source, tag, comm);
        request.type = mpi_manager::receive_request_type(source, tag + 1, comm);
        request.key = mpi_manager::receive_int(source, tag + 2, comm);
        request.value = mpi_manager::receive_string(source, tag + 3, comm);
        return request;
    }
};

struct NodeRequest {
    RequestType type{};
    int key{};
    string value;
    TwoPC state{};

    static void send_node_request(const NodeRequest& request, const int dest, const int tag, MPI_Comm comm) {
        mpi_manager::send_enum(request.type, dest, tag, comm);
        mpi_manager::send_int(request.key, dest, tag + 1, comm);
        mpi_manager::send_string(request.value, dest, tag + 2, comm);
        mpi_manager::send_enum(request.state, dest, tag + 3, comm);
    }

    static NodeRequest receive_node_request(const int source, const int tag, MPI_Comm comm) {
        NodeRequest request;
        request.type = mpi_manager::receive_request_type(source, tag, comm);
        request.key = mpi_manager::receive_int(source, tag + 1, comm);
        request.value = mpi_manager::receive_string(source, tag + 2, comm);
        request.state = mpi_manager::receive_phase_type(source, tag + 3, comm);
        return request;
    }
};

inline RequestType selectType(const int &x) {
    RequestType ret = {};
    switch (x) {
        case 1:
            ret = CREATE;
        break;
        case 2:
            ret = READ;
        break;
        case 3:
            ret = UPDATE;
        break;
        case 4:
            ret = DELETE;
        break;
        default: ;
    }
    return ret;
}

inline TwoPC selectPhase(const int &x) {
    TwoPC ret = {};
    switch (x) {
        case 1:
            ret = PREPARE;
        break;
        case 2:
            ret = COMMIT;
        break;
        case 3:
            ret = ROLLBACK;
        break;
        default: ;
    }
    return ret;
}

#endif //COMMON_H