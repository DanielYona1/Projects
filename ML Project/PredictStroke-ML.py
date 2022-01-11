import numpy as np
import pandas as pd
from matplotlib import  pyplot as plt
import seaborn as sns
import sklearn
from sklearn import metrics
from pandas.api.types import CategoricalDtype
from sklearn.compose import ColumnTransformer
from sklearn.preprocessing import OneHotEncoder
from sklearn.preprocessing import LabelEncoder
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from imblearn.over_sampling import SMOTE
from sklearn.linear_model import LogisticRegression
from sklearn.neighbors import KNeighborsClassifier
from sklearn.model_selection import GridSearchCV
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, roc_auc_score, ConfusionMatrixDisplay, precision_score, recall_score, roc_curve, plot_roc_curve, auc, precision_recall_curve, plot_precision_recall_curve, average_precision_score
from sklearn.model_selection import cross_val_score
from mlxtend.feature_selection import SequentialFeatureSelector
import warnings
warnings.filterwarnings("ignore")

data  = pd.read_csv('/content/drive/MyDrive/Colab Notebooks/train_strokes.csv')

# 0-4, 5-12, 13-18, 19-25, 26-40, 41-60, >60
def group_ages(age):
    if age <= 4:
        return '0-4'
    if 5<=age<=12:
        return '5-12'
    if 13<=age<=18:
        return '13-18'
    if 19<=age<=25:
        return '19-25'
    if 26<=age<=40:
        return '26-40'
    if 41<=age<=60:
        return '41-60'
    else:
        return '>60'


data['group_age'] = data['age'].apply(group_ages)                               # create a new column named group_age (like column 'age') and execute group_ages on each row
age_categories = ['0-4','5-12','13-18','19-25','26-40','41-60','>60']           # define the group ages

age_category = CategoricalDtype(categories = age_categories, ordered=True)
data['group_age'] = data['group_age'].astype(age_category)                      # set the column to be of group age category


data.loc[(data['group_age']=='0-4'),'smoking_status'] = data.loc[(data['group_age']=='0-4'),'smoking_status'].fillna('never smoked')                  # to avoid waste of data, we assume that patients related to these group ages have never smoked
data.loc[(data['group_age']=='5-12'),'smoking_status'] = data.loc[(data['group_age']=='5-12'),'smoking_status'].fillna('never smoked')

data.loc[:,['smoking_status']] = data.loc[:,['smoking_status']].fillna('no record')                                                                   # here we fill the rest of the empty cells with a new group called 'no record'

avg_bmi_groupage = data.groupby('group_age')['bmi'].mean().apply(lambda m: round(m,2))

data.loc[(data['group_age']=='0-4'),'bmi'] = data.loc[(data['group_age']=='0-4'),'bmi'].fillna(avg_bmi_groupage.loc['0-4'])                           # fill the empty cells of bmi column with the mean bmi according to the group age
data.loc[(data['group_age']=='5-12'),'bmi'] = data.loc[(data['group_age']=='5-12'),'bmi'].fillna(avg_bmi_groupage.loc['5-12'])
data.loc[(data['group_age']=='13-18'),'bmi'] = data.loc[(data['group_age']=='13-18'),'bmi'].fillna(avg_bmi_groupage.loc['13-18'])
data.loc[(data['group_age']=='19-25'),'bmi'] = data.loc[(data['group_age']=='19-25'),'bmi'].fillna(avg_bmi_groupage.loc['19-25'])
data.loc[(data['group_age']=='26-40'),'bmi'] = data.loc[(data['group_age']=='26-40'),'bmi'].fillna(avg_bmi_groupage.loc['26-40'])
data.loc[(data['group_age']=='41-60'),'bmi'] = data.loc[(data['group_age']=='41-60'),'bmi'].fillna(avg_bmi_groupage.loc['41-60'])
data.loc[(data['group_age']=='>60'),'bmi'] = data.loc[(data['group_age']=='>60'),'bmi'].fillna(avg_bmi_groupage.loc['>60'])

data.drop(data.loc[data['gender']=='Other'].index, inplace=True)                # only 11 cells with that value under gender column, drop them

data.drop('id', 1, inplace=True)
data.drop('group_age', 1, inplace = True)

correlation = data.corr()                                                       # show correlation matrix to the numerical (not categorized) features
fig, axes = plt.subplots(figsize=(7, 7))
sns.heatmap(correlation, vmax=.8, cbar=True, annot=True, square=True, annot_kws={'size': 10}, linecolor='red')

X_data = data.iloc[:,0:-1].values                                               # separate data to matrix X and vector
y = data.iloc[:, -1].values

plt.figure(figsize=(12,10))

sns.distplot(data[data['stroke'] == 0]["age"], color='grey')
sns.distplot(data[data['stroke'] == 1]["age"], color='blue')

plt.title('No Stroke vs Stroke by Age', fontsize=15)
plt.xlim([18,100])
plt.show()

plt.figure(figsize=(12,5))
sns.boxplot(y='avg_glucose_level', x='stroke' ,data=data)

plt.figure(figsize=(12,5))
plt.title("Average glucose level by age - Stroke vs No Stroke")
sns.scatterplot(x='age', y='avg_glucose_level', hue='stroke', data=data, palette='YlOrRd');

encoder = LabelEncoder()                                                        # for binary features (2 classes)
X_data[:, 0] = encoder.fit_transform(X_data[:, 0])                              # gender column
X_data[:, 4] = encoder.fit_transform(X_data[:, 4])                              # ever_married column
X_data[:, 6] = encoder.fit_transform(X_data[:, 6])                              # Residence_type column


category_encoder = ColumnTransformer(transformers= [('encoder', OneHotEncoder(), [5,9])], remainder = 'passthrough')        # for categorized features with more than one class

X_data = np.array(category_encoder.fit_transform(X_data))

print(X_data.shape)

X_train, X_test, y_train, y_test = train_test_split(X_data, y, test_size=0.33, random_state=42)                             # split data to train and test

scaler = StandardScaler()                                                       # normalize values
X_train = scaler.fit_transform(X_train)
X_test = scaler.fit_transform(X_test)


smote = SMOTE(random_state=42)                                                  # inject random matching values to avoid overfitting
X_train, y_train = smote.fit_resample(X_train, y_train.ravel())                

print (X_train.shape)
print (y_train.shape)
print (sum(y_train == 1))
print (sum(y_train == 0))

#######################
# LOGISTIC REGRESSION #
#######################

logisticreg = LogisticRegression().fit(X_train, y_train)                        # perform logistic regression on the data
predictions = logisticreg.predict(X_test)                                       # predict for test

conf_mat = metrics.confusion_matrix(y_test, predictions)                        # calculate confusion matrix
score = cross_val_score(LogisticRegression(), X_train, y_train)                 # calculate error per example
precision = precision_score(y_test, predictions)
roc = roc_auc_score(y_test, predictions)
recall = recall_score(y_test, predictions)

print ('LogisticRegression\'s score is', score.mean())                          # print total accuracy for all examples
print ('--')
print ('Precision score is ', precision)
print ('--')
print ('ROC Score is', roc)
print ('--')
print ('Recall Score is ', recall)

plt.figure(figsize = (8, 5))
sns.heatmap(conf_mat, cmap = 'Blues', annot = True, fmt = 'd', linewidths = 5, cbar = False, annot_kws = {'fontsize': 15}, 
            yticklabels = ['No stroke', 'Stroke'], xticklabels = ['Predicted no stroke', 'Predicted stroke'])
plt.yticks(rotation = 0)
plt.show()

predictions_prob = logisticreg.predict_proba(X_test)[:,1]                       # calculate probabilities of the predictions

FPR_lr, TPR_lr, threshold = roc_curve(y_test, predictions_prob)                       # calculate roc curve 

plt.figure(figsize = (8, 8))                                                    # show roc curve's graph
plt.plot([0,1],[0,1],"k--",'r-')
figsize=(16,16)
plt.plot(FPR_lr,TPR_lr,color = 'red', label = 'AUC = %0.3f' % roc)
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.title(" Logistic Regression ROC Curve")
plt.legend()
plt.show()

log_reg_params = {"penalty": ['l1', 'l2'], 'C': [0.001, 0.01, 0.025, 0.05, 1]}      # set series of parameters to pass to the model and find the best parameters to get the best score
grid_log_reg = GridSearchCV(LogisticRegression(), log_reg_params, scoring = 'accuracy')
grid_log_reg.fit(X_train, y_train)
best_score = grid_log_reg.best_score_
best_params = grid_log_reg.best_params_
print ('Best Score is',best_score * 100, '%')
print ('Best Parameters is', best_params)

#######################
#         KNN         #
#######################


knn = KNeighborsClassifier().fit(X_train, y_train)                              # perform adaboost on the data
predictions = knn.predict(X_test)

conf_mat = metrics.confusion_matrix(y_test, predictions)                        # calculate roc curve
score = cross_val_score(KNeighborsClassifier(), X_train, y_train)       
precision = precision_score(y_test, predictions)
roc = roc_auc_score(y_test, predictions)
recall = recall_score(y_test, predictions)

print ('K-Nearest Neighbors\' score is', score.mean())
print ('--')
print ('Precision score is ', precision)
print ('--')
print ('ROC Score is', roc)
print ('--')
print ('Recall Score is ', recall)

plt.figure(figsize = (8, 5))
sns.heatmap(conf_mat, cmap = 'Greys', annot = True, fmt = 'd', linewidths = 5, cbar = False, annot_kws = {'fontsize': 15}, 
            yticklabels = ['No stroke', 'Stroke'], xticklabels = ['Predicted no stroke', 'Predicted stroke'])
plt.yticks(rotation = 0)
plt.show()

predictions_prob = knn.predict_proba(X_test)[:,1]                               # calculate probabilities of the predictions

FPR_knn,TPR_knn,threshold = roc_curve(y_test,predictions_prob)

plt.figure(figsize = (8, 8))
plt.plot([0,1],[0,1],"k--",'r-')
figsize=(16,16)
plt.plot(FPR_knn,TPR_knn,color = 'green', label = 'AUC = %0.3f' % roc)
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.title(" K-Nearest Neighbors ROC Curve")
plt.legend()
plt.show()

knn_params = {'n_neighbors':[3,5,7], 'weights': ['uniform', 'distance']}     # set series of parameters to pass to the model and find the best parameters to get the best score 
grid_knn = GridSearchCV(KNeighborsClassifier(), knn_params, scoring = 'accuracy')
grid_knn.fit(X_train, y_train)
best_score = grid_knn.best_score_
best_params = grid_knn.best_params_
print ('Best Score is',best_score * 100, '%')
print ('Best Parameters is', best_params)

#######################
#    RANDOM FOREST    #
#######################


randomforest = RandomForestClassifier().fit(X_train, y_train)                   # perform random forest on the data (default estimator = 100)
predictions = randomforest.predict(X_test)                                      # predict results on test

conf_mat = metrics.confusion_matrix(y_test, predictions)                        # calculate the confusion matrix
score = cross_val_score(RandomForestClassifier(), X_train, y_train, cv=6)     
precision = precision_score(y_test, predictions)
roc = roc_auc_score(y_test, predictions)
recall = recall_score(y_test, predictions)

print ('Random Forest\'s score is', score.mean())
print ('--')
print ('Precision score is ', precision)
print ('--')
print ('ROC Score is', roc)
print ('--')
print ('Recall Score is ', recall)

plt.figure(figsize = (8, 5))
sns.heatmap(conf_mat, cmap = 'Reds', annot = True, fmt = 'd', linewidths = 5, cbar = False, annot_kws = {'fontsize': 15}, 
            yticklabels = ['No stroke', 'Stroke'], xticklabels = ['Predicted no stroke', 'Predicted stroke'])
plt.yticks(rotation = 0)
plt.show()

predictions_prob = randomforest.predict_proba(X_test)[:,1]                      # calculate probabilities of the predictions

FPR_rf,TPR_rf,threshold=roc_curve(y_test,predictions_prob)

plt.figure(figsize = (8, 8))
plt.plot([0,1],[0,1],"k--",'r-')
figsize=(16,16)
plt.plot(FPR_rf,TPR_rf,color = 'blue', label = 'AUC = %0.3f' % roc)
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.title(" Random Forest ROC Curve")
plt.legend()
plt.show()

randomf_params = {'n_estimators':[1000],'max_depth':[5]}                 # set series of parameters to pass to the model and find the best parameters to get the best score
grid_randomf = GridSearchCV(RandomForestClassifier(), randomf_params, scoring = 'accuracy',cv = 6)
grid_randomf.fit(X_train, y_train)
best_score = grid_randomf.best_score_
best_params = grid_randomf.best_params_
print ('Best Score is',best_score * 100)
print ('Best Parameters is', best_params)

#######################
#       ADABOOST      #
#######################


adaboost = AdaBoostClassifier().fit(X_train, y_train)                           # perform adaboost on the data
predictions = adaboost.predict(X_test)                                          # predict for the test

conf_mat = metrics.confusion_matrix(y_test, predictions)
score = cross_val_score(AdaBoostClassifier(), X_train, y_train, cv = 6)
precision = precision_score(y_test, predictions)
roc = roc_auc_score(y_test, predictions)
recall = recall_score(y_test, predictions)

print ('Adaboost\'s score is', score.mean())
print ('--')
print ('Precision score is ', precision)
print ('--')
print ('ROC Score is', roc)
print ('--')
print ('Recall Score is ', recall)

plt.figure(figsize = (8, 5))
sns.heatmap(conf_mat, cmap = 'Greens', annot = True, fmt = 'd', linewidths = 5, cbar = False, annot_kws = {'fontsize': 15}, 
            yticklabels = ['No stroke', 'Stroke'], xticklabels = ['Predicted no stroke', 'Predicted stroke'])
plt.yticks(rotation = 0)
plt.show()

predictions_prob = adaboost.predict_proba(X_test)[:,1]                          # calculate probabilities of the predictions

FPR_ad,TPR_ad,threshold = roc_curve(y_test,predictions_prob)                          # calculate roc curve

plt.figure(figsize = (8, 8))
plt.plot([0,1],[0,1],"k--",'r-')
figsize=(16,16)
plt.plot(FPR_ad,TPR_ad,color = 'purple', label = 'AUC = %0.3f' % roc)
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.title(" AdaBoost ROC Curve")
plt.legend()
plt.show()

plt.figure(figsize = (8, 8))                                                    # show roc-curves of all 4 algorithms for comparison
plt.plot([0,1],[0,1],"k--",'r-')
figsize=(16,16)
plt.plot(FPR_lr,TPR_lr,color = 'red', label = 'ROC_LR')
plt.plot(FPR_knn,TPR_knn,color = 'green', label = 'ROC_KNN')
plt.plot(FPR_rf,TPR_rf,color = 'blue', label = 'ROC_RF')
plt.plot(FPR_ad,TPR_ad,color = 'purple', label = 'ROC_AD')
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.title(" All algorithms ROC Curve")
plt.legend()
plt.show()

knn = KNeighborsClassifier(n_neighbors=5)

feature_selection = SequentialFeatureSelector(knn, k_features=5)                # find 5 most significant features from the data
feature_selection.fit(X_train, y_train)                           
SequentialFeatureSelector(estimator=KNeighborsClassifier(n_neighbors=5),k_features=5) # run KNN again on best features

print(feature_selection.subsets_)