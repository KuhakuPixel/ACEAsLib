#include "ACE/proc_stat.hpp"
#include "CLI11.hpp"
#include "ACE/attach_client.hpp"
#include "ACE/engine_server.hpp"
#include "ACE/input.hpp"
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

const int port = 56666;

void list_ps_cmd_handler() {

  // list running processes
  std::vector<struct proc_info> processes_infos = list_processes();
  // display each of processes
  for (size_t i = 0; i < processes_infos.size(); i++) {
    printf("%d %s\n", processes_infos[i].pid,
           processes_infos[i].proc_name.c_str());
  }
}

void attach_cmd_handler(int pid) {
  if (!proc_is_running(pid)) {
    printf("no process with pid of %d exist\n", pid);
    return;
  }

  // start an engine server that attach to process [pid]
  // starting another thread because starting server will block main thread
  std::thread server_thread = std::thread(

      [=]() {
        //
        engine_server_start(pid, port);
      }

  );

  attach_client client = attach_client(port);
  //
  run_input_loop(
      [&](std::string input) -> E_loop_statement {
        // stop the server when input is "die"
        if (input == "quit") {
          client.stop_server();
          server_thread.join();
          return E_loop_statement::break_;
        }
        // [input] is the command that we sent to ACE
        std::string reply = client.request(input);
        printf("ACE output: %s\n", reply.c_str());
        return E_loop_statement::continue_;
      },
      "ACE");
}
int main(int argc, char **argv) {
  // ==========================
  CLI::App app{"Using ACE as lib"};
  // ========= create list_ps command
  CLI::App *list_ps_cmd =
      app.add_subcommand("list_ps", "list running processes");
  list_ps_cmd->callback([]() { list_ps_cmd_handler(); });

  // ========= create attach command
  CLI::App *attach_cmd =
      app.add_subcommand("attach", "attach ACE to running proceses");
  int pid = -1;
  attach_cmd->add_option("<PID>", pid, "pid of processes to attach")
      ->required();
  attach_cmd->callback([&pid]() { attach_cmd_handler(pid); });
  // =========================
  // parse args
  CLI11_PARSE(app, argc, argv);

  return 0;
}
