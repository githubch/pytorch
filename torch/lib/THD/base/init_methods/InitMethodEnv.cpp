#include "InitMethodEnv.hpp"

namespace thd {

namespace {

constexpr char RANK_ENV[] = "RANK";
constexpr char WORLD_SIZE_ENV[] = "WORLD_SIZE";
constexpr char MASTER_PORT_ENV[] = "MASTER_PORT";
constexpr char MASTER_ADDR_ENV[] = "MASTER_ADDR";

std::tuple<port_type, rank_type> load_master_env() {
  auto port = convertToPort(std::stoul(must_getenv(MASTER_PORT_ENV)));

  rank_type world_size = std::stoul(must_getenv(WORLD_SIZE_ENV));
  if (world_size == 0)
    throw std::domain_error(std::string(WORLD_SIZE_ENV) + " env variable cannot be 0");

  return std::make_tuple(port, world_size);
}


std::tuple<std::string, port_type> load_worker_env() {
  std::string full_address = std::string(must_getenv(MASTER_ADDR_ENV));
  auto found_pos = full_address.rfind(":");
  if (found_pos == std::string::npos)
    throw std::domain_error("invalid master address, usage: IP:PORT | HOSTNAME:PORT");

  std::string str_port = full_address.substr(found_pos + 1);
  auto port = convertToPort(std::stoul(str_port));
  return std::make_tuple(full_address.substr(0, found_pos), port);
}

rank_type load_rank_env() {
  return convertToRank(std::stol(must_getenv(RANK_ENV)));
}

rank_type load_world_size_env() {
  return convertToRank(std::stol(must_getenv(WORLD_SIZE_ENV)));
}

} // anonymous namespace

InitMethodEnv::InitMethodEnv() {}
InitMethodEnv::~InitMethodEnv() {}

InitMethod::Config InitMethodEnv::getConfig() {
  InitMethod::Config config;
  config.rank = load_rank_env();
  if (config.rank == 0) {
    config.master.world_size = load_world_size_env();
    std::tie(config.master.listen_port, config.master.world_size) = load_master_env();
    std::tie(config.master.listen_socket, std::ignore) = listen(config.master.listen_port);
  } else {
    std::tie(config.worker.address, config.worker.port) = load_worker_env();
  }
  return config;
}

} // namespace thd