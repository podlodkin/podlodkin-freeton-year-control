#include <boost/algorithm/hex.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <nil/crypto3/zk/components/blueprint.hpp>
#include <nil/crypto3/zk/components/blueprint_variable.hpp>

#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark.hpp>
#include <nil/crypto3/zk/snark/schemes/ppzksnark/r1cs_gg_ppzksnark/marshalling.hpp>

#include <nil/crypto3/zk/snark/algorithms/generate.hpp>
#include <nil/crypto3/zk/snark/algorithms/verify.hpp>
#include <nil/crypto3/zk/snark/algorithms/prove.hpp>

#include "./comparerLogic.h"

using namespace std;

using namespace nil::crypto3::zk::components;
using namespace nil::crypto3::zk::snark;


ushort INVALID_PROOF_RETURN_CODE = 200;

std::string convert_byteblob_to_hex_string(std::vector<std::uint8_t> blob) {
    // convert byte_blob to hex string and print it to output
    std::string hex;
    hex.reserve(blob.size() * 2);
    boost::algorithm::hex(blob.begin(), blob.end(), back_inserter(hex));
    return hex;
}

void save_byteblob(std::vector<std::uint8_t> byteblob, boost::filesystem::path fname) {
    boost::filesystem::ofstream out(fname);
    for (const auto &v : byteblob) {
        out << v;
    }
    out.close();
}

std::vector<std::uint8_t> load_byteblob(boost::filesystem::path fname) {
    boost::filesystem::ifstream stream(fname, std::ios::in | std::ios::binary);
    std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    if (contents.size() == 0) {
        throw std::ios_base::failure("Empty file");
    }
    return contents;
}


// proving key

void save_proving_key(scheme_type::proving_key_type pk, boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = nil::marshalling::verifier_input_serializer_tvm<scheme_type>::process(pk);
    save_byteblob(byteblob, fname);
}

scheme_type::proving_key_type load_proving_key(boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = load_byteblob(fname);
    nil::marshalling::status_type processingStatus = nil::marshalling::status_type::success;
    return nil::marshalling::verifier_input_deserializer_tvm<scheme_type>::proving_key_process(
        byteblob.cbegin(),
        byteblob.cend(),
        processingStatus);
}

// verification key

void save_verification_key(scheme_type::verification_key_type vk, boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = nil::marshalling::verifier_input_serializer_tvm<scheme_type>::process(vk);
    save_byteblob(byteblob, fname);
}

scheme_type::verification_key_type load_verification_key(boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = load_byteblob(fname);
    nil::marshalling::status_type processingStatus = nil::marshalling::status_type::success;
    return nil::marshalling::verifier_input_deserializer_tvm<scheme_type>::verification_key_process(
        byteblob.cbegin(),
        byteblob.cend(),
        processingStatus);
}

// proof

void save_proof(scheme_type::proof_type proof, boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = nil::marshalling::verifier_input_serializer_tvm<scheme_type>::process(proof);
    save_byteblob(byteblob, fname);
}

scheme_type::proof_type load_proof(boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = load_byteblob(fname);
    nil::marshalling::status_type processingStatus = nil::marshalling::status_type::success;
    return nil::marshalling::verifier_input_deserializer_tvm<scheme_type>::proof_process(
        byteblob.cbegin(),
        byteblob.cend(),
        processingStatus);
}


//  primary input

void save_primary_input(zk::snark::r1cs_primary_input<field_type> primary_input, boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = nil::marshalling::verifier_input_serializer_tvm<scheme_type>::process(primary_input);
    save_byteblob(byteblob, fname);
}

zk::snark::r1cs_primary_input<field_type> load_primary_input(boost::filesystem::path fname) {
    std::vector<std::uint8_t> byteblob = load_byteblob(fname);
    nil::marshalling::status_type processingStatus = nil::marshalling::status_type::success;
    return nil::marshalling::verifier_input_deserializer_tvm<scheme_type>::primary_input_process(
        byteblob.cbegin(),
        byteblob.cend(),
        processingStatus);
}


int setup_keys(boost::filesystem::path pk_path, boost::filesystem::path vk_path) {

    blueprint<field_type> bp;
    ComparerLogic comparerLogic(bp);
    comparerLogic.generate_r1cs_constraints(bp);

    cout << "Blueprint size: " << bp.num_variables() << endl;
    cout << "Generating constraint system..." << endl;
    const r1cs_constraint_system<field_type> constraint_system = bp.get_constraint_system();
    cout << "Number of R1CS constraints: " << constraint_system.num_constraints() << endl;

    cout << "Generating keypair..." << endl;
    scheme_type::keypair_type keypair = generate<scheme_type>(constraint_system);

    cout << "Saving proving key to a file " << pk_path<< endl;
    save_proving_key(keypair.first, pk_path);

    cout << "Saving verification key to a file " << vk_path << endl;
    save_verification_key(keypair.second, vk_path);

    return 0;
}

int create_proof(boost::filesystem::path pk_path, boost::filesystem::path proof_path, boost::filesystem::path pi_path, int minYear, int maxYear, int year) {

    cout << "Loading proving key from a file " << pk_path << endl;
    typename scheme_type::proving_key_type pk = load_proving_key(pk_path);

    blueprint<field_type> bp;
    ComparerLogic comparerLogic(bp);

    cout << "Generating constraint system..." << endl;
    comparerLogic.generate_r1cs_constraints(bp);

    cout << "Generating witness..." << endl;
    comparerLogic.generate_r1cs_witness(bp, minYear, maxYear, year);

    cout << "Blueprint is satisfied: " << bp.is_satisfied() << endl;

    if (!bp.is_satisfied()) {
        return INVALID_PROOF_RETURN_CODE;
    }

    cout << "Generating proof..." << endl;
    const scheme_type::proof_type proof = prove<scheme_type>(pk, bp.primary_input(), bp.auxiliary_input());

    cout << "Saving proof to file " << proof_path << endl;
    save_proof(proof, proof_path);

    cout << "Saving primary input to file " << pi_path << endl;
    save_primary_input(bp.primary_input(), pi_path);
    return 0;
}

int verify_proof(boost::filesystem::path proof_path, boost::filesystem::path vk_path, boost::filesystem::path pi_path) {

    cout << "Loading proof from a file " << proof_path << endl;
    typename scheme_type::proof_type proof = load_proof(proof_path);

    cout << "Loading primary input from a file " << pi_path << endl;
    r1cs_primary_input<field_type> input = load_primary_input(pi_path);

    cout << "Loading verification key from a file " << vk_path << endl;
    typename scheme_type::verification_key_type vk = load_verification_key(vk_path);

    // verify
    using basic_proof_system = r1cs_gg_ppzksnark<curve_type>;
    const bool verified = verify<basic_proof_system>(vk, input, proof);
    cout << "Verification status: " << verified << endl;

    return verified ? 0 : INVALID_PROOF_RETURN_CODE;
}

int main(int argc, char *argv[]) {
    int maxYear, minYear, year;
    boost::filesystem::path pk_path, vk_path, proof_path, pi_path;
    // bool hexFlag;

    boost::program_options::options_description options("CLI Proof Generator");
    options.add_options()
    // ("hex,h", boost::program_options::bool_switch(&hexFlag), "print only hex proof to output")
    ("minYear,minYear", boost::program_options::value<int>(&minYear)->default_value(0))
    ("maxYear,maxYear", boost::program_options::value<int>(&maxYear)->default_value(100))
    ("year,year", boost::program_options::value<int>(&year)->default_value(18))
    ("proving-key-path,pk", boost::program_options::value<boost::filesystem::path>(&pk_path)->default_value("proving.key"))
    ("verification-key-path,vk", boost::program_options::value<boost::filesystem::path>(&vk_path)->default_value("verification.key"))
    ("proof-path,p", boost::program_options::value<boost::filesystem::path>(&proof_path)->default_value("proof"))
    ("primary-input-path,pi", boost::program_options::value<boost::filesystem::path>(&pi_path)->default_value("primary.input"));

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(options).run(), vm);
    boost::program_options::notify(vm);

    cout << setprecision(16);

    if (!argv[1]) {
        cout << "Please select a command: [setup/prove/verify]" << endl;
        return 0;
    }
    else if (string(argv[1]) == "setup") {
        // Generate proving.key & verification.key
        return setup_keys(pk_path, vk_path);
    } else if (string(argv[1]) == "prove") {
        return create_proof(pk_path, proof_path, pi_path, minYear, maxYear, year);
    } else if (string(argv[1]) == "verify") {
        return verify_proof(proof_path, pi_path, vk_path);
    }

    return 0;
}

