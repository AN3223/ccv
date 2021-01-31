#include "case.h"
#include "ccv_case.h"
#include "ccv_nnc_case.h"
#include <ccv.h>
#include <nnc/ccv_nnc.h>
#include <nnc/ccv_nnc_easy.h>

TEST_SETUP()
{
	ccv_nnc_init();
}

TEST_CASE("compare smooth L1")
{
	ccv_nnc_symbolic_graph_t* const graph = ccv_nnc_symbolic_graph_new();
	// batch size = 2, dim = 3.
	const ccv_nnc_tensor_symbol_t a = ccv_nnc_tensor_symbol_new(graph, CPU_TENSOR_NHWC(32F, 2, 3), "a");
	const ccv_nnc_tensor_symbol_t label = ccv_nnc_tensor_symbol_new(graph, CPU_TENSOR_NHWC(32F, 2, 3), "label");
	const ccv_nnc_tensor_symbol_t loss0 = ccv_nnc_tensor_symbol_new(graph, ccv_nnc_tensor_auto, "loss0");
	ccv_nnc_graph_exec_symbol_new(graph, CMD_SMOOTH_L1_FORWARD(1), TENSOR_SYMBOL_LIST(a, label), TENSOR_SYMBOL_LIST(loss0), "smooth l1");
	ccv_nnc_graph_exec_symbol_autogen(graph, 0, 0, CCV_NNC_AUTOGEN_ALL_EXECS | CCV_NNC_AUTOGEN_SOURCES_AND_DESTINATIONS);
	ccv_nnc_symbolic_graph_backward(graph, TENSOR_SYMBOL_LIST(loss0), TENSOR_SYMBOL_LIST(a), SYMBOLIC_GRAPH_SOURCES(graph), SYMBOLIC_GRAPH_DESTINATIONS(graph));
	const ccv_nnc_tensor_symbol_t dloss0 = ccv_nnc_tensor_symbol_for_backward(graph, loss0);
	const ccv_nnc_tensor_symbol_t da0 = ccv_nnc_tensor_symbol_for_backward(graph, a);
	ccv_nnc_graph_exec_symbol_new(graph, CMD_SET_FORWARD(1), 0, 0, TENSOR_SYMBOL_LIST(dloss0), "set 1");
	ccv_nnc_graph_exec_symbol_autogen(graph, 0, 0, CCV_NNC_AUTOGEN_ALL_EXECS | CCV_NNC_AUTOGEN_SOURCES_AND_DESTINATIONS);
	SYMBOLIC_GRAPH_GEN(graph, CCV_NNC_LONG_DOT_GRAPH);
	ccv_nnc_graph_t* run_graph;
	ccv_nnc_tensor_arena_t* tensor_arena;
	ccv_nnc_graph_exec_arena_t* graph_exec_arena;
	ccv_nnc_symbolic_graph_compile(graph, ccv_nnc_default_compile_params,
		0, 0,
		TENSOR_SYMBOL_LIST(da0, loss0),
		SYMBOLIC_GRAPH_SOURCES(graph), SYMBOLIC_GRAPH_DESTINATIONS(graph),
		&run_graph, &tensor_arena, &graph_exec_arena);
	ccv_nnc_tensor_t* const a_tensor = ccv_nnc_tensor_from_symbol(tensor_arena, a);
	a_tensor->data.f32[0] = -0.0001;
	a_tensor->data.f32[1] = 1;
	a_tensor->data.f32[2] = -1;
	a_tensor->data.f32[3] = -0.5;
	a_tensor->data.f32[4] = 0.4;
	a_tensor->data.f32[5] = 0;
	ccv_nnc_tensor_t* const label_tensor = ccv_nnc_tensor_from_symbol(tensor_arena, label);
	label_tensor->data.f32[0] = 0;
	label_tensor->data.f32[1] = 0;
	label_tensor->data.f32[2] = 0;
	label_tensor->data.f32[3] = 0;
	label_tensor->data.f32[4] = 0;
	label_tensor->data.f32[5] = 0;
	ccv_nnc_graph_run(run_graph, 0, TRAVERSE_FULL, 0, 0);
	ccv_nnc_tensor_t* const da0_tensor = ccv_nnc_tensor_from_symbol(tensor_arena, da0);
	ccv_nnc_tensor_t* const da1_tensor = ccv_nnc_tensor_new(0, CPU_TENSOR_NHWC(32F, 2, 3), 0);
	da1_tensor->data.f32[0] = -1;
	da1_tensor->data.f32[1] = 1;
	da1_tensor->data.f32[2] = -1;
	da1_tensor->data.f32[3] = -0.5;
	da1_tensor->data.f32[4] = 0.4;
	da1_tensor->data.f32[5] = 0;
	REQUIRE_TENSOR_EQ(da0_tensor, da1_tensor, "two tensors from combined op and separate ops should be equal");
	ccv_nnc_tensor_t* const loss0_tensor = ccv_nnc_tensor_from_symbol(tensor_arena, loss0);
	ccv_nnc_tensor_t* const loss1_tensor = ccv_nnc_tensor_new(0, CPU_TENSOR_NHWC(32F, 2, 1), 0);
	loss1_tensor->data.f32[0] = 1.5001;
	loss1_tensor->data.f32[1] = 0.205;
	REQUIRE_TENSOR_EQ(loss0_tensor, loss1_tensor, "two tensors from combined op and separate ops should be equal");
	ccv_nnc_graph_free(run_graph);
	ccv_nnc_tensor_arena_free(tensor_arena);
	ccv_nnc_graph_exec_arena_free(graph_exec_arena);
	ccv_nnc_symbolic_graph_free(graph);
	ccv_nnc_tensor_free(da1_tensor);
	ccv_nnc_tensor_free(loss1_tensor);
}

#include "case_main.h"
