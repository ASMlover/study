// Copyright (c) 2025 ASMlover. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list ofconditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materialsprovided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#include <boost/graph/adjacency_list.hpp>
#include <tuple>
#include <utility>
#include <algorithm>
#include <iterator>
#include <iostream>

static void boost_graph_with_four_vertices() noexcept {
  std::cout << "--------- [graph.with_four_vertices] ---------" << std::endl;

  boost::adjacency_list<> g;
  boost::adjacency_list<>::vertex_descriptor v1 = boost::add_vertex(g);
  boost::adjacency_list<>::vertex_descriptor v2 = boost::add_vertex(g);
  boost::adjacency_list<>::vertex_descriptor v3 = boost::add_vertex(g);
  boost::adjacency_list<>::vertex_descriptor v4 = boost::add_vertex(g);

  std::cout << "[demo.graph] " << __func__ << " => " << v1 << "," << v2 << "," << v3 << "," << v4 << std::endl;
}

static void boost_graph_accessing_vertices() noexcept {
  std::cout << "--------- [graph.accessing_vertices] ---------" << std::endl;

  boost::adjacency_list<> g;

  boost::add_vertex(g);
  boost::add_vertex(g);
  boost::add_vertex(g);
  boost::add_vertex(g);

  std::pair<boost::adjacency_list<>::vertex_iterator, boost::adjacency_list<>::vertex_iterator> vs = boost::vertices(g);

  std::copy(vs.first, vs.second,
      std::ostream_iterator<boost::adjacency_list<>::vertex_descriptor>{std::cout, "\n"});
}

static void boost_graph_accessing_edges() noexcept {
  std::cout << "--------- [graph.accessing_edges] ---------" << std::endl;

  boost::adjacency_list<> g;
  boost::adjacency_list<>::vertex_descriptor v1 = boost::add_vertex(g);
  boost::adjacency_list<>::vertex_descriptor v2 = boost::add_vertex(g);
  boost::add_vertex(g);
  boost::add_vertex(g);

  std::pair<boost::adjacency_list<>::edge_descriptor, bool> p = boost::add_edge(v1, v2, g);
  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  p = boost::add_edge(v1, v2, g);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  p = boost::add_edge(v2, v1, g);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  std::pair<boost::adjacency_list<>::edge_iterator, boost::adjacency_list<>::edge_iterator> es = boost::edges(g);
  std::copy(es.first, es.second, std::ostream_iterator<boost::adjacency_list<>::edge_descriptor>{std::cout, "\n"});
}

static void boost_graph_with_selectors() noexcept {
  std::cout << "--------- [graph.with_selectors] ---------" << std::endl;
  using graph = boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS>;

  graph g;
  boost::adjacency_list<>::vertex_descriptor v1 = boost::add_vertex(g);
  boost::adjacency_list<>::vertex_descriptor v2 = boost::add_vertex(g);
  boost::add_vertex(g);
  boost::add_vertex(g);

  std::pair<graph::edge_descriptor, bool> p = boost::add_edge(v1, v2, g);
  std::cout.setf(std::ios::boolalpha);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  p = boost::add_edge(v1, v2, g);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  p = boost::add_edge(v2, v1, g);
  std::cout << "[demo.graph] " << __func__ << " p.second => " << p.second << std::endl;

  std::pair<graph::edge_iterator, graph::edge_iterator> es = boost::edges(g);
  std::copy(es.first, es.second, std::ostream_iterator<graph::edge_descriptor>{std::cout, "\n"});
}

static void boost_graph_creating_indexes() noexcept {
  std::cout << "--------- [grap.creating_indexes] ---------" << std::endl;
  using graph = boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS>;

  enum { topLeft, topRight, bottomRight, bottomLeft};

  graph g;
  boost::add_edge(topLeft, topRight, g);
  boost::add_edge(topRight, bottomRight, g);
  boost::add_edge(bottomRight, bottomLeft, g);
  boost::add_edge(bottomLeft, topLeft, g);

  graph::edge_iterator it, end;
  std::tie(it, end) = boost::edges(g);
  std::copy(it, end,
      std::ostream_iterator<graph::edge_descriptor>{std::cout, "\n"});
}

void boost_graph() noexcept {
  std::cout << "========= [graph] =========" << std::endl;

  boost_graph_with_four_vertices();
  boost_graph_accessing_vertices();
  boost_graph_accessing_edges();
  boost_graph_with_selectors();
  boost_graph_creating_indexes();
}
