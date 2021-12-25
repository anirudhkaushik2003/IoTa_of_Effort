import matplotlib.pyplot as plt
import numpy as np
import requests
import json
from datetime import datetime

timestamp = []

def estimate_coef(x, y):
    # number of observations/points
    n = np.size(x)
 
    # mean of x and y vector
    m_x = np.mean(x)
    m_y = np.mean(y)
 
    # calculating cross-deviation and deviation about x
    SS_xy = np.sum(y*x) - n*m_y*m_x
    SS_xx = np.sum(x*x) - n*m_x*m_x
 
    # calculating regression coefficients
    b_1 = SS_xy / SS_xx
    b_0 = m_y - b_1*m_x
 
    return (b_0, b_1)

def plot_regression_line(x, y, b):
    # plotting the actual points as scatter plot
    plt.scatter(x, y, color = "m",
               marker = "o", s = 30)
 
    # predicted response vector
    y_pred = b[0] + b[1]*x
 
    # plotting the regression line
    plt.plot(x, y_pred, color = "g")
 
    # putting labels
    plt.xlabel('TimeStamp')
    plt.ylabel('volume wasted')
 
    # function to show plot
    plt.show()

def main():
    # observations / data
    
    ae = "HC_SR04_IoTa_of_Effort"
    cnt = "volumeNode"


    # step 1
    headers = {
        'X-M2M-Origin' : 'admin:admin',
        'Content-type' : 'application/json'
    }



    req = requests.get(f'http://3222-103-199-182-12.ngrok.io/~/in-cse/in-name/{ae}/{cnt}/?rcn=4', headers=headers)
    result = json.loads(req.text)

    vals = []        # list of values
    times = []       # time list


    for var in result['m2m:cnt']['m2m:cin']:
        vals.append(float(var['con']))
        times.append(datetime.strptime(var['ct'], '%Y%m%dT%H%M%S'))
        timestamp.append(datetime.strptime(var['ct'], '%Y%m%dT%H%M%S'))
        
    for i in range(len(times)):
        times[i] = int(times[i].second+times[i].minute+times[i].hour)
    
    times = np.array(times)
    vals = np.array(vals)
    # estimating coefficients
    b = estimate_coef(times, vals)
    print("Estimated coefficients:\nb_0 = {}  \
          \nb_1 = {}".format(b[0], b[1]))
 
    # plotting regression line
    plot_regression_line(times, vals, b)
 
if __name__ == "__main__":
    main()