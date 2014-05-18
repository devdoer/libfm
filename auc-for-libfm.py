import sys
def auc( l ):
    total_nonclk = 0
    total_clk = 0
    total_area = 0.0
    for item in l:
        show = item[0]
        clk = item[1]
        nonclk = show - clk
        area = 0.5*(total_clk + total_clk + clk)*nonclk
        total_area += area
        total_clk += clk
        total_nonclk += nonclk
    return total_area / (total_clk * total_nonclk)

def scoreClickAUC(num_clicks, num_shows, predicted_ctr,):
    """
    Calculates the area under the ROC curve (AUC) for click rates

    Parameters
    ----------
    num_clicks : a list containing the number of clicks

    num_shows : a list containing the number of show

    predicted_ctr : a list containing the predicted click-through rates

    Returns
    -------
    auc : the area under the ROC curve (AUC) for click rates
    """
    i_sorted = sorted(range(len(predicted_ctr)),key=lambda i: predicted_ctr[i],
                      reverse=True)
    auc_temp = 0.0
    click_sum = 0.0
    old_click_sum = 0.0
    no_click = 0.0
    no_click_sum = 0.0

    # treat all instances with the same predicted_ctr as coming from the
    # same bucket
    last_ctr = predicted_ctr[i_sorted[0]] + 1.0

    for i in range(len(predicted_ctr)):
        if last_ctr != predicted_ctr[i_sorted[i]]:
            auc_temp += (click_sum+old_click_sum) * no_click / 2.0
            old_click_sum = click_sum
            no_click = 0.0
            last_ctr = predicted_ctr[i_sorted[i]]
        no_click += num_shows[i_sorted[i]] - num_clicks[i_sorted[i]]
        no_click_sum += num_shows[i_sorted[i]] - num_clicks[i_sorted[i]]
        click_sum += num_clicks[i_sorted[i]]
    auc_temp += (click_sum+old_click_sum) * no_click / 2.0
    total_area = click_sum * no_click_sum
    if total_area == 0: auc = 1
    else:
        auc = auc_temp / total_area
    return auc

if __name__ == '__main__':
    l = []
    clks = []
    shows = []
    ctrs = []
    users = {}
    for line in sys.stdin:
        flds = line.split( )
        show = 1
        if flds[1] == '-1':
            clk = 0
        else:
            clk = 1
        q = float( flds[0] )

        shows.append(  show )
        clks.append( clk )
        ctrs.append(  q )

    print  'auc :', scoreClickAUC(clks, shows, ctrs  )
