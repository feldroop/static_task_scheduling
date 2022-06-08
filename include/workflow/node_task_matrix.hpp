#pragma once

#include <vector>

#include <cluster/cluster.hpp>
#include <workflow/task.hpp>

// this is just a vector<vector> with a function call syntax
// for error prevention
template <typename T>
struct node_task_matrix {
private:
    // data[node_id][task_id]
    std::vector<std::vector<T>> const data;

public:
    node_task_matrix(std::vector<std::vector<T>> data_)
        : data(std::move(data_)) {}
    
    inline const T & operator()(
        workflow::task_id const t_id,
        cluster::node_id const n_id
    ) const {
        return data.at(n_id).at(t_id);
    }
};
