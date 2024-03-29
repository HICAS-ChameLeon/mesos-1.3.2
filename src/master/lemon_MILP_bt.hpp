////
//* Copyright  ：SIAT 异构智能计算体系结构与系统研究中心
//* Author     ：Lele Li lilelr@163.com
//* Date       ：2021-7-16
//* Description：Mix-integer linear programming
////

#ifndef CHAMELEON_LEMON_MILP_HPP
#define CHAMELEON_LEMON_MILP_HPP
#include <iostream>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
// google
#include <glog/logging.h>
//#include <gflags/gflags.h>
// lemon
#include <lemon/lp.h>

using std::make_shared;
using std::set;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

namespace chameleon {

class BTLinearModel
{
public:
  explicit BTLinearModel(
    const string& name,
    const double coe_cpu,
    const double coe_mem,
    const double c)
    : m_name(name), m_coe_cpu(coe_cpu), m_coe_mem(coe_mem), m_c(c)
  {}

  int reduced_cores;
  int reduced_executors;
  int reduced_mem;
  int reduced_per_mem;
  int cores_max;
  int per_executor_cores;
  int per_executor_memory;
  int overall_executors;

  const string m_name;
  double m_coe_cpu;
  double m_coe_mem;
  double m_c;

  int lower_bound_reduced_cores;
  int lower_bound_reduced_mem;

  ~BTLinearModel() {}

  template <typename... Args>
  static std::string str_format(const std::string& format, Args... args)
  {
    auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
    std::unique_ptr<char[]> buf(new (std::nothrow) char[size_buf]);

    if (!buf) return std::string("");

    std::snprintf(buf.get(), size_buf, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size_buf - 1);
  }

  string info()
  {
    string result = str_format(
      "model name is %s, m_coe_cpu is %lf, m_coe_mem is %lf, m_c is %lf.\n"
      "overall_executors is %d,  per_executor_cores is %d, per_executor_memory "
      "is %d. \n"
      "reduced_cores is %d,  reduced_executors is %d, reduced_mem is %d, "
      "reduced_per_mem is %d.",
      m_name.c_str(),
      m_coe_cpu,
      m_coe_mem,
      m_c,
      overall_executors,
      per_executor_cores,
      per_executor_memory,
      reduced_cores,
      reduced_executors,
      reduced_mem,
      reduced_per_mem);
    //            LOG(INFO)<<result;
    return result;
  }
};

class MILP
{
public:
  explicit MILP() {}

  static void insert_new_lp_model(const string& name)
  {
    using namespace lemon;
    // Create an instance of the default MIP solver class
    // (it will represent an "empty" problem at first)
    Mip mip;
    // Add two columns (variables) to the problem
    Mip::Col x1 = mip.addCol();
    Mip::Col x2 = mip.addCol();
    // Add rows (constraints) to the problem
    //    mip.addRow(x1 - 5 <= x2);
    mip.addRow(x1 + x2 == 14);
    mip.addRow(0 <= 2 * x1 + x2 <= 25);

    // Set lower and upper bounds for the columns (variables)
    mip.colLowerBound(x1, 0);
    mip.colUpperBound(x2, 10);

    // Set the type of the columns
    mip.colType(x1, Mip::INTEGER);
    mip.colType(x2, Mip::REAL);

    // Specify the objective function
    mip.max();
    mip.obj(5 * x1 + 3 * x2);

    // Solve the problem using the underlying MIP solver
    mip.solve();
    // Print the results
    if (mip.type() == Mip::OPTIMAL) {
      std::cout << "Objective function value: " << mip.solValue() << std::endl;
      std::cout << "x1 = " << mip.sol(x1) << std::endl;
      std::cout << "x2 = " << mip.sol(x2) << std::endl;
    } else {
      std::cout << "Optimal solution not found." << std::endl;
    }
    // large input datasets
    if (name.find("LDA") != std::string::npos) {
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel lda(
          "LDA", -3.79712601e-03, -6.59352184e-06, 0.05952716033460187);
        lda.cores_max = 156;
        lda.per_executor_cores = 5;
        lda.per_executor_memory = 2795;
        lda.overall_executors = lda.cores_max / lda.per_executor_cores;
        lda.lower_bound_reduced_cores = lda.per_executor_cores * 1;
        lda.lower_bound_reduced_mem = lda.per_executor_memory * 1;

        m_bt_lps.insert({name, lda});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("TeraSort") != std::string::npos) {
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel terasort(
          "TeraSort", -0.14457721, 0.00019771, 0.022838498050515695);
        terasort.cores_max = 128;
        terasort.per_executor_cores = 9;
        terasort.per_executor_memory = 6561;
        terasort.overall_executors =
          terasort.cores_max / terasort.per_executor_cores;
        terasort.lower_bound_reduced_cores = terasort.per_executor_cores * 1;
        terasort.lower_bound_reduced_mem = terasort.per_executor_memory * 1;
        m_bt_lps.insert({name, terasort});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("Gradient") != std::string::npos) {
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel gbt("GBT", -0.08423214, 0.00019774, -0.11438166924927734);
        gbt.cores_max = 161;
        gbt.per_executor_cores = 4;
        gbt.per_executor_memory = 1563;
        gbt.overall_executors = gbt.cores_max / gbt.per_executor_cores;
        gbt.lower_bound_reduced_cores = gbt.per_executor_cores * 1;
        gbt.lower_bound_reduced_mem = gbt.per_executor_memory * 1;
        m_bt_lps.insert({name, gbt});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("ALS") != std::string::npos) {
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel als(
          "ALS", 3.81821507e-02, -6.46756197e-05, 0.2715613015300328);
        als.cores_max = 178;
        als.per_executor_cores = 2;
        als.per_executor_memory = 1384;
        als.overall_executors = als.cores_max / als.per_executor_cores;
        als.lower_bound_reduced_cores = als.per_executor_cores * 1;
        als.lower_bound_reduced_mem = als.per_executor_memory * 1;
        m_bt_lps.insert({name, als});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("SVD") != std::string::npos) {
      //           // SVD
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel svd(
          "SVD", -1.23448008e-01, 2.69642447e-05, 0.08060215473953322);
        svd.cores_max = 119;
        svd.per_executor_cores = 2;
        svd.per_executor_memory = 7826;
        svd.overall_executors = svd.cores_max / svd.per_executor_cores;
        svd.lower_bound_reduced_cores = svd.per_executor_cores * 1;
        svd.lower_bound_reduced_mem = svd.per_executor_memory * 1;
        m_bt_lps.insert({name, svd});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("DenseKMeans") != std::string::npos) {
      //           // K-means
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel kmeans(
          "DenseKMeans", 1.66599047e-02, -1.32982637e-05, 0.09324776331922635);
        kmeans.cores_max = 163;
        kmeans.per_executor_cores = 2;
        kmeans.per_executor_memory = 3077;
        kmeans.overall_executors = kmeans.cores_max / kmeans.per_executor_cores;
        kmeans.lower_bound_reduced_cores = kmeans.per_executor_cores * 1;
        kmeans.lower_bound_reduced_mem = kmeans.per_executor_memory * 1;
        m_bt_lps.insert({name, kmeans});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("WordCount") != std::string::npos) {
      //           // wordcount
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel word(
          "WordCount", -1.49027704e-02, -1.32982637e-05, 4.30475388e-05);
        word.cores_max = 172;
        word.per_executor_cores = 9;
        word.per_executor_memory = 2622;
        word.overall_executors = word.cores_max / word.per_executor_cores;
        word.lower_bound_reduced_cores = word.per_executor_cores * 1;
        word.lower_bound_reduced_mem = word.per_executor_memory * 1;
        m_bt_lps.insert({name, word});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("ScalaSort") != std::string::npos) {
      //           // Sort
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel sort(
          "ScalaSort", -0.55890257, 0.00097112, -0.19602875588436175);
        sort.cores_max = 61;
        sort.per_executor_cores = 6;
        sort.per_executor_memory = 3456;
        sort.overall_executors = sort.cores_max / sort.per_executor_cores;
        sort.lower_bound_reduced_cores = sort.per_executor_cores * 1;
        sort.lower_bound_reduced_mem = sort.per_executor_memory * 1;
        m_bt_lps.insert({name, sort});
        LOG(INFO) << "insert a model named " << name;
      }
    } else if (name.find("NaiveBayes") != std::string::npos) {
      //           // NaiveBayes
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel bayes(
          "NaiveBayes", -0.06814784, 0.00024497, -0.038757380707594546);
        bayes.cores_max = 172;
        bayes.per_executor_cores = 9;
        bayes.per_executor_memory = 2622;
        bayes.overall_executors = bayes.cores_max / bayes.per_executor_cores;
        bayes.lower_bound_reduced_cores = bayes.per_executor_cores * 1;
        bayes.lower_bound_reduced_mem = bayes.per_executor_memory * 1;
        m_bt_lps.insert({name, bayes});
        LOG(INFO) << "insert a model named " << name;
      }
    }else if (name.find("SVM") != std::string::npos) {
      //           // NaiveBayes
      if (m_bt_lps.count(name) == 0) {
        BTLinearModel bayes(
          "SVM", -0.06740706, 0.00011015, 0.1282460042419834);
        bayes.cores_max = 131;
        bayes.per_executor_cores = 4;
        bayes.per_executor_memory = 4578;
        bayes.overall_executors = bayes.cores_max / bayes.per_executor_cores;
        bayes.lower_bound_reduced_cores = bayes.per_executor_cores * 1;
        bayes.lower_bound_reduced_mem = bayes.per_executor_memory * 1;
        m_bt_lps.insert({name, bayes});
        LOG(INFO) << "insert a model named " << name;
      }
    }


    // huge input datasets
    //            if(name.find("LDA")!= std::string::npos){
    //                if(m_bt_lps.count("LDA")==0){
    //                    BTLinearModel
    //                    lda("LDA",2.32329538e-02,1.85709874e-05,-0.4778566673203253);
    //                    lda.cores_max = 156;
    //                    lda.per_executor_cores =  5;
    //                    lda.per_executor_memory = 2795;
    //                    lda.overall_executors = lda.cores_max /
    //                    lda.per_executor_cores; lda.lower_bound_reduced_cores
    //                    = lda.per_executor_cores *2;
    //                    lda.lower_bound_reduced_mem = lda.per_executor_memory
    //                    *2;
    //
    //                    m_bt_lps.insert({name, lda});
    //                    LOG(INFO)<<"insert a model named "<<name;
    //                }
    //            }else if(name.find("TeraSort")!= std::string::npos){
    //                if(m_bt_lps.count(name)==0){
    //                    BTLinearModel
    //                    terasort("TeraSort",-0.0238,0.00016533,0.09906666666666664);
    //                    terasort.cores_max = 128;
    //                    terasort.per_executor_cores = 9;
    //                    terasort.per_executor_memory = 6561;
    //                    terasort.overall_executors = terasort.cores_max /
    //                    terasort.per_executor_cores;
    //                    terasort.lower_bound_reduced_cores =
    //                    terasort.per_executor_cores *2;
    //                    terasort.lower_bound_reduced_mem =
    //                    terasort.per_executor_memory *2;
    //                    m_bt_lps.insert({name, terasort});
    //                    LOG(INFO)<<"insert a model named "<<name;
    //
    //                }
    //            }else if(name.find("Gradient")!= std::string::npos){
    //                if(m_bt_lps.count(name)==0){
    //                    BTLinearModel
    //                    gbt("GBT",-6.15300123e-03,1.79400630e-06,0.1331771159919384);
    //                    gbt.cores_max = 161;
    //                    gbt.per_executor_cores = 4;
    //                    gbt.per_executor_memory = 1563;
    //                    gbt.overall_executors = gbt.cores_max /
    //                    gbt.per_executor_cores; gbt.lower_bound_reduced_cores
    //                    = gbt.per_executor_cores *2;
    //                    gbt.lower_bound_reduced_mem = gbt.per_executor_memory
    //                    *2; m_bt_lps.insert({name, gbt}); LOG(INFO)<<"insert a
    //                    model named "<<name;
    //
    //                }
    //            }else if(name.find("ALS")!= std::string::npos){
    //                if(m_bt_lps.count(name)==0){
    //                    BTLinearModel
    //                    als("ALS",-1.48646513e-02,-8.91943875e-07,0.2490425115114014);
    //                    als.cores_max = 178;
    //                    als.per_executor_cores = 2;
    //                    als.per_executor_memory = 1384;
    //                    als.overall_executors = als.cores_max /
    //                    als.per_executor_cores; als.lower_bound_reduced_cores
    //                    = als.per_executor_cores *2;
    //                    als.lower_bound_reduced_mem = als.per_executor_memory
    //                    *2; m_bt_lps.insert({name, als}); LOG(INFO)<<"insert a
    //                    model named "<<name;
    //
    //                }
    //            }
  }

  static unordered_map<string, BTLinearModel> create_bts()
  {
    unordered_map<string, BTLinearModel> result;
    return result;
  }

  static unordered_map<string, BTLinearModel> m_bt_lps;
  static bool m_ILP_solution;
};

unordered_map<string, BTLinearModel> MILP::m_bt_lps = MILP::create_bts();
bool MILP::m_ILP_solution = false;
/**
 *
 * @param lc_name latency-critical framework name, like "repartition"
 * @param cpus: the targeted cpus to be reduced
 * @param mem: the targeted amount of memory to be reduced in MB
 * @param bt_names: the vector recording the registered framework names, most of
 * them are BT jobs
 * @return The BTLinearModel with reduced cores and Spark executors as the
 * caculated result of ILP
 */
vector<BTLinearModel> mix_integer_linear_programming_two(
  const string& lc_name,
  const int cpus,
  const int mem,
  const vector<string>& bt_names)
{
  LOG(INFO) << "mix_integer_linear_programming";

  vector<BTLinearModel> result;
  using namespace lemon;
  // two BT workloads
  if (bt_names.size() == 2) {
    if (
      MILP::m_bt_lps.count(bt_names[0]) && MILP::m_bt_lps.count(bt_names[1])) {
      BTLinearModel& model1 = MILP::m_bt_lps.at(bt_names[0]);
      BTLinearModel& model2 = MILP::m_bt_lps.at(bt_names[1]);
      LOG(INFO) << model1.m_name << " " << model2.m_name;
      // Create an instance of the default MIP solver class
      // (it will represent an "empty" problem at first)
      Mip mip;
      // Add two columns (variables) to the problem
      Mip::Col a1 = mip.addCol();
      Mip::Col a2 = mip.addCol();
      Mip::Col a3 = mip.addCol();
      Mip::Col a4 = mip.addCol();
      // Add rows (constraints) to the problem
      LOG(INFO) << "lele target cpus and memory are " << cpus << " " << mem;
      mip.addRow(a1 + a3 == cpus);
      mip.addRow(a2 + a4 == mem);
      // Set lower and upper bounds for the columns (variables)
      mip.colLowerBound(a1, model1.lower_bound_reduced_cores);
      mip.colLowerBound(a3, model2.lower_bound_reduced_cores);

      mip.colLowerBound(a2, model1.lower_bound_reduced_mem);
      mip.colLowerBound(a4, model2.lower_bound_reduced_mem);
      //
      mip.colUpperBound(a1, 20);
      mip.colUpperBound(a3, 20);


      // Set the type of the columns
      mip.colType(a1, Mip::INTEGER);
      mip.colType(a2, Mip::INTEGER);
      mip.colType(a3, Mip::INTEGER);
      mip.colType(a4, Mip::INTEGER);

      // Specify the objective function
      mip.min();
      mip.obj(
        model1.m_coe_cpu * a1 + model1.m_coe_mem * a2 + model1.m_c +
        model2.m_coe_cpu * a3 + model2.m_coe_mem * a4 + model2.m_c);
      // Solve the problem using the underlying MIP solver
      mip.solve();
      // Print the results
      if (mip.type() == Mip::OPTIMAL) {
        LOG(INFO) << "leleObjective function value: " << mip.solValue();
        LOG(INFO) << "model1 name is " << model1.m_name;
        LOG(INFO) << "model2 name is " << model2.m_name;
        LOG(INFO) << "a1 = " << mip.sol(a1);

        model1.reduced_cores = mip.sol(a1);
        model1.reduced_executors =
          int(ceil(model1.reduced_cores / model1.per_executor_cores));
        LOG(INFO) << "model1.reduced_executors is " << model1.reduced_executors;
        LOG(INFO) << "a2 = " << mip.sol(a2);
        model1.reduced_mem = mip.sol(a2);
        LOG(INFO) << "model1 reduced_mem is " << model1.reduced_mem
                  << " per_executor_memory is " << model1.per_executor_memory
                  << " overall execuctors is " << model1.overall_executors;

        model1.reduced_per_mem =
          (model1.reduced_mem -
           model1.per_executor_memory * model1.reduced_executors) /
          (model1.overall_executors - model1.reduced_executors);
        LOG(INFO) << " model1.reduced_per_mem is " << model1.reduced_per_mem;

        LOG(INFO) << "a3 = " << mip.sol(a3);
        model2.reduced_cores = mip.sol(a3);
        model2.reduced_executors =
          model2.reduced_cores / model2.per_executor_cores;
        LOG(INFO) << "model2.reduced_executors is " << model2.reduced_executors;
        LOG(INFO) << "a4 = " << mip.sol(a4);
        model2.reduced_mem = mip.sol(a4);
        LOG(INFO) << "model2 reduced_mem is " << model2.reduced_mem
                  << " per_executor_memory is " << model2.per_executor_memory
                  << " overall execuctors is " << model2.overall_executors;
        model2.reduced_per_mem =
          (model2.reduced_mem -
           model2.per_executor_memory * model2.reduced_executors) /
          (model2.overall_executors - model2.reduced_executors);
        LOG(INFO) << " model2.reduced_per_mem is " << model2.reduced_per_mem;
        LOG(INFO) << "The estimated performance losses of two models are ";
        LOG(INFO) << "model1 is "
                  << (model1.m_coe_cpu * mip.sol(a1) +
                      model1.m_coe_mem * mip.sol(a2) + model1.m_c);
        LOG(INFO) << "model2 is "
                  << (model2.m_coe_cpu * mip.sol(a3) +
                      model2.m_coe_mem * mip.sol(a4) + model2.m_c);
        result.push_back(model1);
        result.push_back(model2);

        LOG(INFO) << model1.info();
        LOG(INFO) << model2.info();
        MILP::m_ILP_solution = true;
      } else {
        LOG(INFO) << "Optimal solution not found.";
      }

    } else {
      LOG(INFO) << "cannot find the specified model!!! ";
    }
  }
  return result;
}

vector<BTLinearModel> mix_integer_linear_programming_two_PARTIES(
  const string& lc_name,
  const int cpus,
  const int mem,
  const vector<string>& bt_names)
{
  LOG(INFO) << "mix_integer_linear_programming";

  vector<BTLinearModel> result;
  using namespace lemon;
  // two BT workloads
  if (bt_names.size() == 2) {
    if (
      MILP::m_bt_lps.count(bt_names[0]) && MILP::m_bt_lps.count(bt_names[1])) {
      int a1,a2,a3,a4;
      a1 = a3 = ceil(cpus/2);
      a2 = a4 = ceil(mem/2);
      BTLinearModel& model1 = MILP::m_bt_lps.at(bt_names[0]);
      BTLinearModel& model2 = MILP::m_bt_lps.at(bt_names[1]);
      LOG(INFO) << model1.m_name << " " << model2.m_name;

      double aggregate_performance_loss =  model1.m_coe_cpu * a1 + model1.m_coe_mem * a2 + model1.m_c +
                                           model2.m_coe_cpu * a3 + model2.m_coe_mem * a4 + model2.m_c;
      // Print the results
        LOG(INFO) << "Objective function value: " <<aggregate_performance_loss;
        LOG(INFO) << "model1 name is " << model1.m_name;
        LOG(INFO) << "model2 name is " << model2.m_name;
        LOG(INFO) << "a1 = " << a1;

        model1.reduced_cores = a1;
        model1.reduced_executors =
          int(ceil(model1.reduced_cores / model1.per_executor_cores));
        LOG(INFO) << "model1.reduced_executors is " << model1.reduced_executors;
        LOG(INFO) << "a2 = " << a2;
        model1.reduced_mem = a2;
        LOG(INFO) << "model1 reduced_mem is " << model1.reduced_mem
                  << " per_executor_memory is " << model1.per_executor_memory
                  << " overall execuctors is " << model1.overall_executors;

        model1.reduced_per_mem =
          (model1.reduced_mem -
           model1.per_executor_memory * model1.reduced_executors) /
          (model1.overall_executors - model1.reduced_executors);
        LOG(INFO) << " model1.reduced_per_mem is " << model1.reduced_per_mem;

        LOG(INFO) << "a3 = " << a3;
        model2.reduced_cores = a3;
        model2.reduced_executors =
          model2.reduced_cores / model2.per_executor_cores;
        LOG(INFO) << "model2.reduced_executors is " << model2.reduced_executors;
        LOG(INFO) << "a4 = " << a4;
        model2.reduced_mem = a4;
        LOG(INFO) << "model2 reduced_mem is " << model2.reduced_mem
                  << " per_executor_memory is " << model2.per_executor_memory
                  << " overall execuctors is " << model2.overall_executors;
        model2.reduced_per_mem =
          (model2.reduced_mem -
           model2.per_executor_memory * model2.reduced_executors) /
          (model2.overall_executors - model2.reduced_executors);
        LOG(INFO) << " model2.reduced_per_mem is " << model2.reduced_per_mem;
        LOG(INFO) << "The estimated performance losses of two models are ";
        LOG(INFO) << "model1 is "
                  << (model1.m_coe_cpu * a1 +
                      model1.m_coe_mem * a2 + model1.m_c);
        LOG(INFO) << "model2 is "
                  << (model2.m_coe_cpu * a3 +
                      model2.m_coe_mem * a4 + model2.m_c);
        result.push_back(model1);
        result.push_back(model2);

        LOG(INFO) << model1.info();
        LOG(INFO) << model2.info();
        MILP::m_ILP_solution = true;

    } else {
      LOG(INFO) << "cannot find the specified model!!! ";
    }
  }
  return result;
}

/**
 *
 * @param lc_name latency-critical framework name, like "repartition"
 * @param cpus: the targeted cpus to be reduced
 * @param mem: the targeted amount of memory to be reduced in MB
 * @param bt_names: the vector recording the registered framework names, most of
 * them are BT jobs
 * @return The BTLinearModel with reduced cores and Spark executors as the
 * caculated result of ILP
 */
vector<BTLinearModel> mix_integer_linear_programming_three(
  const string& lc_name,
  const int cpus,
  const int mem,
  const vector<string>& bt_names)
{
  LOG(INFO) << "mix_integer_linear_programming";

  vector<BTLinearModel> result;
  using namespace lemon;
  // three BT workloads
  if (bt_names.size() == 3) {
    if (
      MILP::m_bt_lps.count(bt_names[0]) && MILP::m_bt_lps.count(bt_names[1])) {
      BTLinearModel& model1 = MILP::m_bt_lps.at(bt_names[0]);
      BTLinearModel& model2 = MILP::m_bt_lps.at(bt_names[1]);
      BTLinearModel& model3 = MILP::m_bt_lps.at(bt_names[2]);
      LOG(INFO) <<"Tree BT workloads are "<< model1.m_name << " " << model2.m_name <<" "<<model3.m_name;
      // Create an instance of the default MIP solver class
      // (it will represent an "empty" problem at first)
      Mip mip;
      // Add two columns (variables) to the problem
      Mip::Col a1 = mip.addCol();
      Mip::Col a2 = mip.addCol();
      Mip::Col a3 = mip.addCol();
      Mip::Col a4 = mip.addCol();
      Mip::Col a5 = mip.addCol();
      Mip::Col a6 = mip.addCol();
      // Add rows (constraints) to the problem
      LOG(INFO) << "lele target cpus and memory are " << cpus << " " << mem;
      mip.addRow(a1 + a3 + a5== cpus);
      mip.addRow(a2 + a4 + a6 == mem);
      // Set lower and upper bounds for the columns (variables)
      mip.colLowerBound(a1, model1.lower_bound_reduced_cores);
      mip.colLowerBound(a3, model2.lower_bound_reduced_cores);
      mip.colLowerBound(a5, model3.lower_bound_reduced_cores);

      mip.colLowerBound(a2, model1.lower_bound_reduced_mem);
      mip.colLowerBound(a4, model2.lower_bound_reduced_mem);
      mip.colLowerBound(a6, model3.lower_bound_reduced_mem);

      //
      mip.colUpperBound(a1, 12);
      mip.colUpperBound(a3, 12);
      mip.colUpperBound(a5, 12);


      // Set the type of the columns
      mip.colType(a1, Mip::INTEGER);
      mip.colType(a2, Mip::INTEGER);
      mip.colType(a3, Mip::INTEGER);
      mip.colType(a4, Mip::INTEGER);
      mip.colType(a5, Mip::INTEGER);
      mip.colType(a6, Mip::INTEGER);
      // Specify the objective function
      mip.min();
      mip.obj(
        model1.m_coe_cpu * a1 + model1.m_coe_mem * a2 + model1.m_c +
        model2.m_coe_cpu * a3 + model2.m_coe_mem * a4 + model2.m_c +
          model3.m_coe_cpu * a5 + model3.m_coe_mem * a6 + model3.m_c );

      // Solve the problem using the underlying MIP solver
      mip.solve();
      // Print the results
      if (mip.type() == Mip::OPTIMAL) {
        LOG(INFO) << "Objective function value: " << mip.solValue();
        LOG(INFO) << "model1 name is " << model1.m_name;
        LOG(INFO) << "model2 name is " << model2.m_name;
        LOG(INFO) << "model3 name is " << model3.m_name;
        LOG(INFO) << "a1 = " << mip.sol(a1);

        model1.reduced_cores = mip.sol(a1);
        model1.reduced_executors =
          model1.reduced_cores / model1.per_executor_cores;
        LOG(INFO) << "model1.reduced_executors is " << model1.reduced_executors;
        LOG(INFO) << "a2 = " << mip.sol(a2);
        model1.reduced_mem = mip.sol(a2);
        LOG(INFO) << "model1 reduced_mem is " << model1.reduced_mem
                  << " per_executor_memory is " << model1.per_executor_memory
                  << " overall execuctors is " << model1.overall_executors;

        model1.reduced_per_mem =
          (model1.reduced_mem -
           model1.per_executor_memory * model1.reduced_executors) /
          (model1.overall_executors - model1.reduced_executors);
        LOG(INFO) << " model1.reduced_per_mem is " << model1.reduced_per_mem;

        LOG(INFO) << "a3 = " << mip.sol(a3);
        model2.reduced_cores = mip.sol(a3);
        model2.reduced_executors =
          model2.reduced_cores / model2.per_executor_cores;
        LOG(INFO) << "model2.reduced_executors is " << model2.reduced_executors;
        LOG(INFO) << "a4 = " << mip.sol(a4);
        model2.reduced_mem = mip.sol(a4);
        LOG(INFO) << "model2 reduced_mem is " << model2.reduced_mem
                  << " per_executor_memory is " << model2.per_executor_memory
                  << " overall execuctors is " << model2.overall_executors;
        model2.reduced_per_mem =
          (model2.reduced_mem -
           model2.per_executor_memory * model2.reduced_executors) /
          (model2.overall_executors - model2.reduced_executors);
        LOG(INFO) << " model2.reduced_per_mem is " << model2.reduced_per_mem;

        LOG(INFO) << "a5 = " << mip.sol(a5);
        model3.reduced_cores = mip.sol(a5);
        model3.reduced_executors =
          model3.reduced_cores / model3.per_executor_cores;
        LOG(INFO) << "model3.reduced_executors is " << model3.reduced_executors;
        LOG(INFO) << "a6 = " << mip.sol(a6);
        model3.reduced_mem = mip.sol(a6);
        LOG(INFO) << "model3 reduced_mem is " << model3.reduced_mem
                  << " per_executor_memory is " << model3.per_executor_memory
                  << " overall execuctors is " << model3.overall_executors;
        model3.reduced_per_mem =
          (model3.reduced_mem -
            model3.per_executor_memory * model3.reduced_executors) /
          (model3.overall_executors - model3.reduced_executors);
        LOG(INFO) << " model3.reduced_per_mem is " << model3.reduced_per_mem;


        LOG(INFO) << "The estimated performance losses of two models are ";
        LOG(INFO) << "model1 is "
                  << (model1.m_coe_cpu * mip.sol(a1) +
                      model1.m_coe_mem * mip.sol(a2) + model1.m_c);
        LOG(INFO) << "model2 is "
                  << (model2.m_coe_cpu * mip.sol(a3) +
                      model2.m_coe_mem * mip.sol(a4) + model2.m_c);
        LOG(INFO) << "model3 is "
                  << (model3.m_coe_cpu * mip.sol(a5) +
                    model3.m_coe_mem * mip.sol(a6) + model3.m_c);
        result.push_back(model1);
        result.push_back(model2);
        result.push_back(model3);


        LOG(INFO) << model1.info();
        LOG(INFO) << model2.info();
        LOG(INFO) << model3.info();

        MILP::m_ILP_solution = true;
      } else {
        LOG(INFO) << "Optimal solution not found.";
      }

    } else {
      LOG(INFO) << "cannot find the specified model!!! ";
    }
  }
  return result;
}


} // namespace chameleon


#endif