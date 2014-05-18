/*
	Stochastic Gradient Descent based learning for classification and regression

	Based on the publication(s):
	Steffen Rendle (2010): Factorization Machines, in Proceedings of the 10th IEEE International Conference on Data Mining (ICDM 2010), Sydney, Australia.

	Author:   Steffen Rendle, http://www.libfm.org/
	modified: 2012-01-04

	Copyright 2010-2012 Steffen Rendle, see license.txt for more information
*/


#ifndef FM_LEARN_SGD_ELEMENT_H_
#define FM_LEARN_SGD_ELEMENT_H_

#include "fm_learn_sgd.h"

class fm_learn_sgd_element: public fm_learn_sgd {
	public:
		virtual void init() {
			fm_learn_sgd::init();

			if (log != NULL) {
				log->addField("rmse_train", std::numeric_limits<double>::quiet_NaN());
			}
		}
		virtual void learn(Data& train, Data& test) {
			//SGD training
			fm_learn_sgd::learn(train, test);//call parent, do nothing, cout

			std::cout << "SGD: DON'T FORGET TO SHUFFLE THE ROWS IN TRAINING DATA TO GET THE BEST RESULTS." << std::endl; 
			// SGD
			for (int i = 0; i < num_iter; i++) {
			
				double iteration_time = getusertime();
				for (train.data->begin(); !train.data->end(); train.data->next()) {
					
					//getRow() return sparse_row<T> of sparse_entry of (id, value)
					double p = fm->predict(train.data->getRow(), sum, sum_sqr);//fm_model* fm defined in fm_model.h
					double mult = 0;
					DATA_FLOAT target = train.data->getRowTarget();
					if (task == 0) {//regression
						p = std::min(max_target, p);
						p = std::max(min_target, p);
						mult = p-target;//Gradient
					} else if (task == 1) {//classification
						mult = target*(1.0/(1.0+exp(-target*p)) - 1.0);//Gradient 
					}				
					SGD(train.data->getRow(), mult, sum);					
				}				
				iteration_time = (getusertime() - iteration_time);
				double rmse_train = evaluate(train);
				double rmse_test = evaluate(test);
				std::cout << "#Iter=" << std::setw(3) << i << "\tTrain=" << rmse_train << "\tTest=" << rmse_test << std::endl;
				if (log != NULL) {
					log->log("rmse_train", rmse_train);
					log->log("time_learn", iteration_time);
					log->newLine();
				}
			}		
		}
		
};

#endif /*FM_LEARN_SGD_ELEMENT_H_*/
