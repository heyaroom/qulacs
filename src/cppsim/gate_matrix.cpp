
#ifndef _MSC_VER
extern "C" {
#include <csim/update_ops.h>
#include <csim/utility.h>
}
#else
#include <csim/update_ops.h>
#include <csim/utility.h>
#endif

#include "state.hpp"
#include "gate_matrix.hpp"
#include "type.hpp"
#include <numeric>
#include <algorithm>

// In construction, "copy" a given matrix. If a given matrix is large, use "move" constructor.
QuantumGateMatrix::QuantumGateMatrix(const std::vector<UINT>& target_qubit_index_list_, const ComplexMatrix& matrix_element, const std::vector<UINT>& control_qubit_index_list_){
    for(auto val : target_qubit_index_list_){
        this->_target_qubit_list.push_back(TargetQubitInfo(val,0));
    }
    for(auto val : control_qubit_index_list_){
        this->_control_qubit_list.push_back(ControlQubitInfo(val,1));
    }
    this->_matrix_element = ComplexMatrix(matrix_element);
    this->_name = "DenseMatrix";
}
QuantumGateMatrix::QuantumGateMatrix(const std::vector<TargetQubitInfo>& target_qubit_index_list_, const ComplexMatrix& matrix_element, const std::vector<ControlQubitInfo>& control_qubit_index_list_){
    this->_target_qubit_list = std::vector<TargetQubitInfo>(target_qubit_index_list_);
    this->_control_qubit_list = std::vector<ControlQubitInfo>(control_qubit_index_list_);
    this->_matrix_element = ComplexMatrix(matrix_element);
    this->_name = "DenseMatrix";
}

// In construction, "move" a given matrix, which surpess the cost of copying large matrix element.
QuantumGateMatrix::QuantumGateMatrix(const std::vector<UINT>& target_qubit_index_list_, ComplexMatrix* matrix_element, const std::vector<UINT>& control_qubit_index_list_){
    for(auto val : target_qubit_index_list_){
        this->_target_qubit_list.push_back(TargetQubitInfo(val,0));
    }
    for(auto val : control_qubit_index_list_){
        this->_control_qubit_list.push_back(ControlQubitInfo(val,1));
    }
    this->_matrix_element.swap(*matrix_element);
    this->_name = "DenseMatrix";
}
QuantumGateMatrix::QuantumGateMatrix(const std::vector<TargetQubitInfo>& target_qubit_index_list_, ComplexMatrix* matrix_element, const std::vector<ControlQubitInfo>& control_qubit_index_list_){
    this->_target_qubit_list = std::vector<TargetQubitInfo>(target_qubit_index_list_);
    this->_control_qubit_list = std::vector<ControlQubitInfo>(control_qubit_index_list_);
    this->_matrix_element.swap(*matrix_element);
    this->_name = "DenseMatrix";
}



void QuantumGateMatrix::update_quantum_state(QuantumStateBase* state) {
    ITYPE dim = 1ULL << state->qubit_count;

    //Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> row_matrix(this->_matrix_element);
    //const CTYPE* matrix_ptr = reinterpret_cast<const CTYPE*>(row_matrix.data());
    const CTYPE* matrix_ptr = reinterpret_cast<const CTYPE*>(this->_matrix_element.data());

    // convert list of QubitInfo to list of UINT
    std::vector<UINT> target_index;
    std::vector<UINT> control_index;
    std::vector<UINT> control_value;
    for(auto val : this->_target_qubit_list){
        target_index.push_back(val.index());
    }
    for(auto val : this->_control_qubit_list){
        control_index.push_back(val.index());
        control_value.push_back(val.control_value());
    }

    // single qubit dense matrix gate
    if(this->_target_qubit_list.size() == 1){

        // no control qubit
        if(this->_control_qubit_list.size() == 0){
            single_qubit_dense_matrix_gate(
                target_index[0],
                matrix_ptr , state->data_c(), dim );
        }
        // single control qubit
        else if(this->_control_qubit_list.size() == 1){
            single_qubit_control_single_qubit_dense_matrix_gate(
                control_index[0], control_value[0],
                target_index[0],
                matrix_ptr, state->data_c(), dim );

        }
        // multiple control qubits
        else{
            multi_qubit_control_single_qubit_dense_matrix_gate(
                control_index.data(), control_value.data(), (UINT)(control_index.size()),
                target_index[0],
                matrix_ptr, state->data_c(), dim );
        }
    }

    // multi qubit dense matrix gate
    else{
        // no control qubit
        if(this->_control_qubit_list.size() == 0){
            multi_qubit_dense_matrix_gate(
                target_index.data(), (UINT)(target_index.size()),
                matrix_ptr, state->data_c(), dim );
        }
        // single control qubit
        else if(this->_control_qubit_list.size() == 1){
            single_qubit_control_multi_qubit_dense_matrix_gate(
                control_index[0], control_value[0],
                target_index.data(), (UINT)(target_index.size()),
                matrix_ptr, state->data_c(), dim );
        }
        // multiple control qubit
        else{
            multi_qubit_control_multi_qubit_dense_matrix_gate(
                control_index.data(), control_value.data(), (UINT)(control_index.size()),
                target_index.data(), (UINT)(target_index.size()),
                matrix_ptr, state->data_c(), dim);
        }
    }
}


void QuantumGateMatrix::add_control_qubit(UINT qubit_index, UINT control_value) {
    this->_control_qubit_list.push_back(ControlQubitInfo(qubit_index, control_value));
    this->_gate_property &= (~FLAG_PAULI);
    this->_gate_property &= (~FLAG_GAUSSIAN);
}

std::string QuantumGateMatrix::to_string() const {
    std::stringstream os;
    os << QuantumGateBase::to_string();
    os << " * Matrix" << std::endl;
    os << this->_matrix_element << std::endl;
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const QuantumGateMatrix& gate) {
    os << gate.to_string();
    return os;
}
std::ostream& operator<<(std::ostream& os, QuantumGateMatrix* gate) {
    os << *gate;
    return os;
}
