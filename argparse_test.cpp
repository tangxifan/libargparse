#include "argparse.hpp"
#include "argparse_util.hpp"
struct Args {
    std::string architecture_file;
    std::string circuit;

    bool disp;
    bool auto_value;

    bool pack;
    bool place;
    bool route;

    bool timing_analysis;
    const char* slack_definition;
    bool echo_files;
    bool verify_file_digests;

    std::string blif_file;
    std::string net_file;
    std::string place_file;
    std::string route_file;
    std::string sdc_file;
    std::string outfile_prefix;

    bool absorb_buffer_luts;
    bool sweep_dangling_primary_ios;
    bool sweep_dangling_nets;
    bool sweep_dangling_blocks;
    bool sweep_constant_primary_outputs;

    bool connection_driven_clustering;
    bool allow_unrelated_clustering;
    float alpha_clustering;
    float beta_clustering;
    bool timing_driven_clustering;
    std::string cluster_seed_type;

    size_t seed;
    bool enable_timing_computations;
    float inner_num;
    float init_t;
    float exit_t;
    float alpha_t;
    std::string fix_pins;
    std::string place_algorithm;
    size_t place_chan_width;

    float timing_tradeoff;
    int recompute_crit_iter;
    int inner_loop_recompute_divider;
    float td_place_exp_first;
    float td_place_exp_last;

    int max_router_iterations;
    float first_iter_pres_fac;
    float initial_pres_fac;
    float pres_fac_mult;
    float acc_fac;
    int bb_factor;
    std::string base_cost_type;
    float bend_cost;
    std::string route_type;
    size_t route_chan_width;
    size_t min_route_chan_width_hint;
    bool verify_binary_search;
    std::string router_algorithm;
    int min_incremental_reroute_fanout;

    float astar_fac;
    float max_criticality;
    float criticality_exp;
    std::string routing_failure_predictor;

    bool power;
    std::string tech_properties_file;
    std::string activity_file;



    bool full_stats;
    bool gen_post_synthesis_netlist;
};

bool expect_pass(argparse::ArgumentParser& parser, std::vector<std::string> cmd_line);
bool expect_fail(argparse::ArgumentParser& parser, std::vector<std::string> cmd_line);

struct OnOff {
    bool from_str(std::string str) {
        if      (str == "on")  return true;
        else if (str == "off") return false;
        std::stringstream msg;
        msg << "Invalid conversion from '" << str << "' to boolean (expected one of: " << argparse::join(default_choices(), ", ") << ")";
        throw argparse::ArgParseConversionError(msg.str());
    }

    std::string to_str(bool val) {
        if (val) return "on";
        return "off";
    }

    std::vector<std::string> default_choices() {
        return {"on", "off"};
    }
};

int main(int argc, const char** argv) {
    Args args;

    auto parser = argparse::ArgumentParser("Test parser for libargparse");
    parser.prog(argv[0]);
    parser.epilog("This is the epilog");

    auto& pos_grp = parser.add_argument_group("positional arguments");
    pos_grp.add_argument(args.architecture_file, "architecture")
            .help("FPGA Architecture description file (XML)");
    pos_grp.add_argument(args.circuit, "circuit")
            .help("Circuit file (or circuit name if --blif_file specified)");

    auto& stage_grp = parser.add_argument_group("stage options");
    stage_grp.add_argument<bool,OnOff>(args.pack, "--pack")
            .help("Run packing")
            .action(argparse::Action::STORE_TRUE)
            .default_value("off");
    stage_grp.add_argument<bool,OnOff>(args.place, "--place")
            .help("Run placement")
            .action(argparse::Action::STORE_TRUE)
            .default_value("off");
    stage_grp.add_argument<bool,OnOff>(args.route, "--route")
            .help("Run routing")
            .action(argparse::Action::STORE_TRUE)
            .default_value("off");
    stage_grp.add_argument<bool,OnOff>(args.route, "--analysis")
            .help("Run analysis")
            .action(argparse::Action::STORE_TRUE)
            .default_value("off");

    stage_grp.epilog("If none of the stage options are specified, all stages are run.\n"
                     "Analysis is always run after routing.");

    auto& gfx_grp = parser.add_argument_group("graphics options");
    gfx_grp.add_argument<bool,OnOff>(args.disp, "--disp")
            .help("Enable or disable interactive graphics")
            .default_value("off");
    gfx_grp.add_argument(args.auto_value, "--auto")
            .help("Controls how often VPR pauses for interactive"
                  " graphics (requiring Proceed to be clicked)."
                  " Higher values pause less frequently")
            .default_value("1")
            .choices({"0", "1", "2"})
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& gen_grp = parser.add_argument_group("general options");
    gen_grp.add_argument<bool,OnOff>(args.timing_analysis, "--timing_analysis")
            .help("Controls whether timing analysis (and timing driven optimizations) are enabled.")
            .default_value("on") ;
    gen_grp.add_argument(args.slack_definition, "--slack_definition")
            .help("Sets the slack definition used by the classic timing analyyzer")
            .default_value("R")
            .choices({"R", "I", "S", "G", "C", "N"})
            .show_in(argparse::ShowIn::HELP_ONLY);
    gen_grp.add_argument<bool,OnOff>(args.echo_files, "--echo_file")
            .help("Generate echo files of key internal data structures."
                  " Useful for debugging VPR, and typically end in .echo")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);
    gen_grp.add_argument<bool,OnOff>(args.verify_file_digests, "--verify_file_digests")
            .help("Verify that files loaded by VPR (e.g. architecture, netlist,"
                  " previous packing/placement/routing) are consistent")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& file_grp = parser.add_argument_group("filename options");
    file_grp.add_argument(args.blif_file, "--blif_file")
            .help("Path to technology mapped circuit in BLIF format")
            .show_in(argparse::ShowIn::HELP_ONLY);
    file_grp.add_argument(args.net_file, "--net_file")
            .help("Path to packed netlist file")
            .show_in(argparse::ShowIn::HELP_ONLY);
    file_grp.add_argument(args.place_file, "--place_file")
            .help("Path to placement file")
            .show_in(argparse::ShowIn::HELP_ONLY);
    file_grp.add_argument(args.route_file, "--route_file")
            .help("Path to routing file")
            .show_in(argparse::ShowIn::HELP_ONLY);
    file_grp.add_argument(args.sdc_file, "--sdc_file")
            .help("Path to timing constraints file in SDC format")
            .show_in(argparse::ShowIn::HELP_ONLY);
    file_grp.add_argument(args.outfile_prefix, "--outfile_prefix")
            .help("Prefix for output files")
            .show_in(argparse::ShowIn::HELP_ONLY);


    auto& netlist_grp = parser.add_argument_group("netlist options");
    netlist_grp.add_argument<bool,OnOff>(args.absorb_buffer_luts, "--absorb_buffer_luts")
            .help("Controls whether LUTS programmed as buffers are absorbed by downstream logic")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    netlist_grp.add_argument<bool,OnOff>(args.sweep_dangling_primary_ios, "--sweep_dangling_primary_ios")
            .help("Controls whether dangling primary inputs and outputs are removed from the netlist")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    netlist_grp.add_argument<bool,OnOff>(args.sweep_dangling_nets, "--sweep_dangling_nets")
            .help("Controls whether dangling nets are removed from the netlist")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    netlist_grp.add_argument<bool,OnOff>(args.sweep_dangling_blocks, "--sweep_dangling_blocks")
            .help("Controls whether dangling blocks are removed from the netlist")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    netlist_grp.add_argument<bool,OnOff>(args.sweep_constant_primary_outputs, "--sweep_constant_primary_outputs")
            .help("Controls whether primary outputs driven by constant values are removed from the netlist")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& pack_grp = parser.add_argument_group("packing options");
    pack_grp.add_argument<bool,OnOff>(args.connection_driven_clustering, "--connection_driven_clustering")
            .help("Controls whether or not packing prioritizes the absorption of nets with fewer"
                  " connections into a complex logic block over nets with more connections")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    pack_grp.add_argument<bool,OnOff>(args.allow_unrelated_clustering, "--allow_unrelated_clustering")
            .help("Controls whether or not primitives with no attraction to the current cluster"
                  " can be packed into it")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    pack_grp.add_argument(args.alpha_clustering, "--alpha_clustering")
            .help("Parameter that weights the optimization of timing vs area. 0.0 focuses solely on"
                  " area, 1.0 solely on timing.")
            .default_value("0.75")
            .show_in(argparse::ShowIn::HELP_ONLY);
    pack_grp.add_argument(args.beta_clustering, "--beta_clustering")
            .help("Parameter that weights the absorption of small nets vs signal sharing."
                  " 0.0 focuses solely on sharing, 1.0 solely on small net absoprtion."
                  " Only meaningful if --connection_driven_clustering=on")
            .default_value("0.9")
            .show_in(argparse::ShowIn::HELP_ONLY);
    pack_grp.add_argument<bool,OnOff>(args.timing_driven_clustering, "--timing_driven_clustering")
            .help("Controls whether custering optimizes for timing")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    pack_grp.add_argument(args.cluster_seed_type, "--cluster_seed_type")
            .help("Controls how primitives are chosen as seeds."
                  " (Default: blend if timing driven, max_inputs otherwise)")
            .choices({"blend", "timing", "max_inputs"})
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& place_grp = parser.add_argument_group("placement options");
    place_grp.add_argument(args.seed, "--seed")
            .help("Placement random number generator seed")
            .default_value("1")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument<bool,OnOff>(args.enable_timing_computations, "--enable_timing_computations")
            .help("Displays delay statistics even if placement is not timing driven")
            .default_value("on")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.inner_num, "--inner_num")
            .help("Controls number of moves per temperature: inner_num * num_blocks ^ (4/3)")
            .default_value("10.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.init_t, "--init_t")
            .help("Initial temperature for manual annealing schedule")
            .default_value("100.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.exit_t, "--exit_t")
            .help("Temperature at which annealing which terminate for manual annealing schedule")
            .default_value("0.01")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.alpha_t, "--alpha_t")
            .help("Temperature scaling factor for manual annealing schedule."
                  " Old temperature is multiplied by alpha_t")
            .default_value("0.01")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.fix_pins, "--fix_pins")
            .help("Fixes I/O pad locations during placement."
                  " Can be 'random' for a random initial assignment,"
                  " 'off' to allow the place to optimize pad locations,"
                  " or a file specifying the pad locations.")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.place_algorithm, "--place_algorithm")
            .help("Controls which placement algorithm is used")
            .default_value("path_timing_driven")
            .choices({"bounding_box", "path_timing_driven"})
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_grp.add_argument(args.place_chan_width, "--place_chan_width")
            .help("Sets the assumed channel width during placement")
            .default_value("100")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& place_timing_grp = parser.add_argument_group("timing-driven placement options");
    place_timing_grp.add_argument(args.timing_tradeoff, "--timing_tradeoff")
            .help("Trade-off control between delay and wirelength during placement."
                  " 0.0 focuses completely on wirelength, 1.0 completely on timing")
            .default_value("0.5")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_timing_grp.add_argument(args.recompute_crit_iter, "--recompute_crit_iter")
            .help("Controls how many temperature updates occur between timing analysis during placement")
            .default_value("1")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_timing_grp.add_argument(args.inner_loop_recompute_divider, "--inner_loop_recompute_divider")
            .help("Controls how many timing analysies are perform per temperature during placement")
            .default_value("0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_timing_grp.add_argument(args.td_place_exp_first, "--td_place_exp_first")
            .help("Controls how critical a connection is as a function of slack at the start of placement."
                  " A value of zero treats all connections as equally critical (regardless of slack)."
                  " Values larger than 1.0 cause low slack connections to be treated more critically."
                  " The value increases to --td_place_exp_last during placement.")
            .default_value("1.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    place_timing_grp.add_argument(args.td_place_exp_last, "--td_place_exp_last")
            .help("Controls how critical a connection is as a function of slack at the end of placement.")
            .default_value("8.0")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& route_grp = parser.add_argument_group("routing options");
    route_grp.add_argument(args.max_router_iterations, "--max_route_iterations")
            .help("Maximum number of Pathfinder-based routing iterations before the circuit is"
                  " declared unroutable at a given channel width")
            .default_value("50")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.initial_pres_fac, "--first_iter_pres_fac")
            .help("Sets the present overuse factor for the first routing iteration")
            .default_value("0.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.initial_pres_fac, "--initial_pres_fac")
            .help("Sets the present overuse factor for the second routing iteration")
            .default_value("0.5")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.pres_fac_mult, "--pres_fac_mult")
            .help("Sets the growth factor by which the present overuse penalty factor is"
                  " multiplied after each routing iteration")
            .default_value("1.3")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.acc_fac, "--acc_fac")
            .help("Specifies the accumulated overuse factor (historical congestion cost factor)")
            .default_value("1.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.bb_factor, "--bb_factor")
            .help("Sets the distance (in channels) outside a connection's bounding box which can be explored")
            .default_value("3")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.base_cost_type, "--base_cost_type")
            .help("Sets the basic cost of routing resource nodes:\n"
                  " * demand_only: based on expected demand of node type\n"
                  " * delay_normalized: like demand_only but normalized to magnitude of typical routing resource delay\n"
                  "(Default: demand_only for bread-first router, delay_normalized for timing-driven router)")
            .choices({"demand_only", "delay_normalized"})
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.bend_cost, "--bend_cost")
            .help("The cost of a bend. (Default: 1.0 for global routing, 0.0 for detailed routing)")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.route_type, "--route_type")
            .help("Specifies whether global, or combined global and detailed routing is performed.")
            .choices({"global", "detailed"})
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.route_chan_width, "--route_chan_width")
            .help("Specifies a fixed channel width to route at.")
            .metavar("CHANNEL_WIDTH");
    route_grp.add_argument(args.min_route_chan_width_hint, "--min_route_chan_width_hint")
            .help("Hint to the router what the minimum routable channel width is."
                  " Good hints can speed-up determining the minimum channel width.")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument<bool,OnOff>(args.verify_binary_search, "--verify_binary_search")
            .help("Force the router to verify the minimum channel width by routing at"
                  " consecutively lower channel widths until two consecutive failures are observed.")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.router_algorithm, "--router_algorithm")
            .help("Specifies the router algorithm to use.\n"
                  " * breadth_first: focuses solely on routability\n"
                  " * timing driven: focuses on routability and circuit speed\n")
            .default_value("timing_driven")
            .choices({"breadth_first", "timing_driven"})
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_grp.add_argument(args.min_incremental_reroute_fanout, "--min_incremental_reroute_fanout")
            .help("The net fanout thershold above which nets will be re-routed incrementally.")
            .default_value("64")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& route_timing_grp = parser.add_argument_group("timing-driven routing options");
    route_timing_grp.add_argument(args.astar_fac, "--astar_fac")
            .help("How aggressive the directed search used by the timing-driven router is."
                  " Values between 1 and 2 are resonable; higher values trade some quality for reduced run-time")
            .default_value("1.2")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_timing_grp.add_argument(args.max_criticality, "--max_criticality")
            .help("Sets the maximum fraction of routing cost derived from delay (vs routability) for any net."
                  " 0.0 means no attention is paid to delay, 1.0 means nets on the critical path ignore congestion")
            .default_value("0.99")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_timing_grp.add_argument(args.criticality_exp, "--criticality_exp")
            .help("Controls the delay-routability trade-off for nets as a function of slack."
                  " 0.0 implies all nets treated equally regardless of slack."
                  " At large values (>> 1) only nets on the critical path will consider delay.")
            .default_value("1.0")
            .show_in(argparse::ShowIn::HELP_ONLY);
    route_timing_grp.add_argument(args.routing_failure_predictor, "--routing_failure_predictor")
            .help("Controls how aggressively the router will predict a routing as unsuccessful"
                  " and give up early. This can significantly reducing the run-time required"
                  " to find the minimum channel width).\n"
                  " * safe: Only abort when it is extremely unlikely a routing will succeed\n"
                  " * aggressive: Further reduce run-time by giving up earlier. This may increase the reported minimum channel width\n"
                  " * off: Only abort when the maximum number of iterations is reached\n")
            .default_value("safe")
            .choices({"safe", "aggressive", "off"})
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& analysis_grp = parser.add_argument_group("analysis options");

    analysis_grp.add_argument<bool,OnOff>(args.full_stats, "--full_stats")
            .help("Print extra statistics about the circuit and it's routing (useful for wireability analysis)")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);

    analysis_grp.add_argument<bool,OnOff>(args.gen_post_synthesis_netlist, "--gen_post_synthesis_netlist")
            .help("Generates the post-synthesis netlist (in BLIF and Verilog) along with delay information (in SDF)."
                  " Used for post-implementation simulation and verification")
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);

    auto& power_grp = parser.add_argument_group("power analysis options");
    power_grp.add_argument<bool,OnOff>(args.power, "--power")
            .help("Enable power estimation")
            .action(argparse::Action::STORE_TRUE)
            .default_value("off")
            .show_in(argparse::ShowIn::HELP_ONLY);
    power_grp.add_argument(args.tech_properties_file, "--tech_properties_file")
            .help("XML file containing CMOS technology properties (see documentation).")
            .show_in(argparse::ShowIn::HELP_ONLY);
    power_grp.add_argument(args.activity_file, "--activity_file")
            .help("Signal activities file for all nets (see documentation).")
            .show_in(argparse::ShowIn::HELP_ONLY);

    parser.print_help();

    //auto specified_args = parser.parse_args(argc, argv);
    //for(auto& arg : specified_args) {
        //std::cout << "Group: " << arg->group_name() << " Specified argument: " << arg->long_option();
        //auto short_opt = arg->short_option();
        //if (!short_opt.empty()) {
            //std::cout << "/" << short_opt;
        //}
        //std::cout << "\n";
    //}

    std::vector<std::vector<std::string>> pass_cases = {
        {"my_arch.xml", "my_circuit.blif"},
        {"my_arch.xml", "my_circuit.blif", "--pack"},
        {"my_arch.xml", "my_circuit.blif", "--timing_analysis", "on"},
        {"my_arch.xml", "my_circuit.blif", "--route_chan_width", "300"},
        {"my_arch.xml", "my_circuit.blif", "--criticality_exp", "2"}, //Float from integer
        {"my_arch.xml", "my_circuit.blif", "--criticality_exp", "2.0"}, //Float
    };

    int num_failed = 0;
    for(const auto& cmd_line : pass_cases) {
        bool pass = expect_pass(parser, cmd_line);

        if(!pass) {
            std::cout << "Failed to parse: '" << argparse::join(cmd_line, " ") << "'" << std::endl;
            ++num_failed;
        }
    }

    std::vector<std::vector<std::string>> fail_cases = {
        {""}, //Missing positional
        {"my_arch.xml"}, //Missing positional
        {"my_arch.xml", "my_circuit.blif", "extra"}, //Extra positional
        {"my_arch.xml", "my_circuit.blif", "--route_chan_width"}, //Missing value to option
        {"my_arch.xml", "my_circuit.blif", "--route_chan_width", "off"}, //Wrong option value
        {"my_arch.xml", "my_circuit.blif", "--disp", "132"}, //Wrong option value
        {"my_arch.xml", "my_circuit.blif", "--route_chan_width", "300", "5"}, //Extra option value
        {"my_arch.xml", "my_circuit.blif", "--pack", "on"}, //Extra option value to toggle option
        {"my_arch.xml", "my_circuit.blif", "--route_chan_width", "300.5"}, //Type mismatch
        {"my_arch.xml", "my_circuit.blif", "--criticality_exp", "on"}, //Wrong value type for float
    };

    for(const auto& cmd_line : fail_cases) {
        bool pass = expect_fail(parser, cmd_line);

        if(!pass) {
            std::cout << "Parsed successfully when expected failure: '" << argparse::join(cmd_line, " ") << "'" << std::endl;
            ++num_failed;
        }
    }

    return num_failed;
}

bool expect_pass(argparse::ArgumentParser& parser, std::vector<std::string> cmd_line) {
    try {
        parser.parse_args(cmd_line);
    } catch(const argparse::ArgParseError& err) {
        std::cout << err.what() << "  [FAIL]" << std::endl;
        return false;
    }
    return true;
}

bool expect_fail(argparse::ArgumentParser& parser, std::vector<std::string> cmd_line) {
    try {
        parser.parse_args(cmd_line);
    } catch(const argparse::ArgParseError& err) {
        std::cout << err.what() << "  [PASS]" << std::endl;
        return true;
    }
    return false;
}
