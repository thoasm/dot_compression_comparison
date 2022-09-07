#!/usr/bin/env python3
import os
import csv
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_pdf import PdfPages


plot_folder = "plots"

plot_infos = [
        {
            #"name": "Sinus dependant vectors",
            "name": "sin_vectors",
            "file": "2022-09-07_1700_sin_results.csv",
        },
        {
            #"name": "Random vectors [0, 1)",
            "name": "rand_vectors",
            "file": "2022-09-07_1700_rand_results.csv",
        },
    ]



### dictionary to match purpose to CSV header
h_dict = {
        "name" : "Compression technique",
        "error": "Absolute error",
        "ratio": "Compression ratio",
    }

def read_csv(path=None):
    """
    Opens the CSV file in 'path' and returns 2 dictionaries:
    1. The key is the precision it was performed in, the value is the list of a list
       of column entries of the csv file (the lines are sorted according to number
       of computations)
    2. The key is the same as in h_dict, the value is the index of the row
       array / list for the correesponding key
    """
    if path == None:
        raise Exception("No filename specified! Unable to read file.")
    with open(path, 'r') as f:
        print("The csv file <{}> is opened".format(path))
        csv_f = csv.reader(f, delimiter=';', skipinitialspace=True)
        header = next(csv_f)
        print("CSV header: {}".format(header))
        
        i_dict = {}
        for key, val in h_dict.items():
            for i in range(len(header)):
                if header[i] == val:
                    i_dict[key] = i
        print("Resulting index dictionary: {}".format(i_dict))

        data = []

        for r in csv_f:
            data.append(r)

    return data, i_dict


############################### Actual Plotting ###############################
### Color definition
myblue    = (0, 0.4470, 0.7410);
myorange  = (0.8500, 0.3250, 0.0980);
myyellow  = (0.9290, 0.6940, 0.1250);
mymagenta = (0.4940, 0.1840, 0.5560);
mygreen   = (0.4660, 0.6740, 0.1880);
mycyan    = (0.3010, 0.7450, 0.9330);
myred     = (0.6350, 0.0780, 0.1840);
myblack   = (0.2500, 0.2500, 0.2500);
mybrown   = (0.6500, 0.1600, 0.1600);

dark_mod = 2
mydarkred     = (0.6350 / dark_mod, 0.0780 / dark_mod, 0.1840 / dark_mod);
mydarkgreen   = (0.4660 / dark_mod, 0.6740 / dark_mod, 0.1880 / dark_mod);
mydarkblue    = (0, 0.4470 / dark_mod, 0.7410 / dark_mod);

### Other globals
LineWidth = 1
MarkerSize = 8


comp_techniques = [
        {
            "prefix": "sz3_1",
            "marker": 'X',
            "color": mygreen,
            "label": "SZ3",
            },
        {
            "prefix": "zfp_1",
            "marker": 'P',
            "color": myblue,
            "label": "ZFP",
            },
    ]


precision_details = {
        "double": {
            "marker": 'X',
            "color": mygreen,
            "label": "fp64",
            },
        "float": {
            "marker": 'P',
            "color": myblue,
            "label": "fp32",
            },
        "Ac<3, d, d>": {
            "marker": 'x',
            "color": myorange,
            "label": "Accessor<fp64, fp64>",
            },
        "Ac<3, d, f>": {
            "marker": '+',
            "color": myyellow,
            "label": "Accessor<fp64, fp32>",
            },
        "Ac<3, d, p32>": {
            "marker": 'D',
            "color": mymagenta,
            "label": "Accessor<fp64, posit32>",
            },
        "Ac<3, f, p16>": {
            "marker": 'o',
            "color": myblack,
            "label": "Accessor<fp32, posit16>",
            },
        }


def create_fig_ax():
    """
    Creates a tuple of figure and axis for future plots.
    The size, the visibility of the grid and the log-scale of x and y is preset
    """
    fig = Figure(figsize=(10, 4)) # Properly garbage collected
    ax = fig.add_subplot()
    #fig, ax = plt.subplots(figsize=(10, 4)) # NOT garbage collected!
    grid_minor_color = (.9, .9, .9)
    grid_major_color = (.8, .8, .8)
    ax.grid(True, which="major", axis="both", linestyle='-', linewidth=1, color=grid_major_color)
    ax.grid(True, which="minor", axis="both", linestyle=':', linewidth=1, color=grid_minor_color)
    #ax.loglog()
    #ax.semilogy()
    return fig, ax


def plot_figure(fig, file_name, plot_prefix):
    """Plots the given figure fig as various formats with a base-name of file_name.
    plot_folder will be used as the filder for the file; plot_prefix will be the
    prefix of each file."""

    file_path = plot_folder + '/' + plot_prefix + file_name
    print("plotting {}...".format(file_path))
    p_bbox = "tight"
    p_pad = 0
    p_dpi = 300  # Only useful for non-scalable formats
    with PdfPages(file_path+".pdf") as export_pdf:
        export_pdf.savefig(fig, dpi=p_dpi, bbox_inches=p_bbox, pad_inches=p_pad)
    fig.savefig(file_path+".svg", dpi=p_dpi, bbox_inches=p_bbox, pad_inches=p_pad, format="svg")
    fig.savefig(file_path+".png", dpi=p_dpi, bbox_inches=p_bbox, pad_inches=p_pad, format="png")


def plot_for_all(ax, comp_tech, x_key, y_key):
    """
    plots given x and y keys for all precisions of interest on the axis ax.
    """
    for tech in comp_tech:
        x_list = [i for i in range(1, len(tech[x_key]) + 1)]
        ax.plot(x_list, tech[y_key], label=tech["label"],
                marker=tech["marker"], color=tech["color"],
                linewidth=LineWidth, markersize=MarkerSize)
        ax.set_xticks(x_list)
        ax.set_xticklabels(tech[x_key])
    """ To get / set x- and y-limits:
    ax.set_xlim(0.7070722721781199, 1449.6396483523677)
    ax.set_ylim(148.24516110946269, 24024.62127583265)
    xl, xr = ax.get_xlim()
    yl, yr = ax.get_ylim()
    print("xlim: ({}, {}); ylim: ({}, {})".format(xl, xr, yl, yr));
    """

def add_table(ax, plot_data, value_key, colLabel, value_lambda):
    table_row_labels = []
    table_vals = []
    for prec in precisions_to_print:
        if prec not in plot_data:
            continue
        data = plot_data[prec]
        if prec in precision_details:
            label = precision_details[prec]["label"]
            value = value_lambda(data[value_key])
            table_row_labels.append(label)
            table_vals.append([value])

    #print(table_row_labels, table_vals)
    new_table = ax.table(cellText=table_vals, rowLabels=table_row_labels,
            colLabels=[colLabel], colWidths=[0.15], cellLoc="right",
            colLoc="left", rowLoc="left", loc="lower center", zorder=3,
            edges='closed')



if __name__ == "__main__":
    bw_color = myblack
    fp64_color = mydarkgreen
    fp32_color = mydarkblue

    # Change to the directory where the script is placed
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    # Make sure the plot folder exists
    if not os.path.exists(plot_folder):
        os.makedirs(plot_folder)

    for plot_info in plot_infos:
        data, i_dict = read_csv(plot_info["file"])


        # Generate data for plotting for all available precisions
        plot_data = {}
        for tech in comp_techniques:
            double_error = 0.0
            point_errors = []
            comp_ratios = []
            dot_errors = []
            for line in data:
                cur_name = line[i_dict["name"]]
                print(tech)
                if cur_name.startswith(tech["prefix"]):
                    point_errors.append(cur_name[cur_name.find('_')+1:])
                    dot_errors.append(float(line[i_dict["error"]]))
                    comp_ratios.append(float(line[i_dict["ratio"]]))
                elif cur_name == "double":
                    double_error = float(line[i_dict["error"]]);
            tech["p_errors"] = point_errors
            tech["d_errors"] = dot_errors
            tech["comp_ratios"] = comp_ratios


        fig, ax = create_fig_ax()
        ax.semilogy()
        plot_for_all(ax, comp_techniques, "p_errors", "d_errors")
        ax.axhline(double_error, linestyle='--', marker='', linewidth=LineWidth,
                color=bw_color, label="No compression")
        ax.set_xlabel("Pointwise absolute error bound")
        ax.set_ylabel("DOT error")
        ax.legend(loc="upper right")
        plot_figure(fig, "_dot_error", plot_info["name"])
        

        fig, ax = create_fig_ax()
        plot_for_all(ax, comp_techniques, "p_errors", "comp_ratios")
        
        ax.axhline(1.0, linestyle='--', marker='', linewidth=LineWidth,
                color=bw_color, label="No compression")
        ax.set_xlabel("Pointwise absolute error bound")
        ax.set_ylabel("Compression ratio")
        ax.legend(loc="upper right")
        plot_figure(fig, "_dot_compression_ratio", plot_info["name"])

