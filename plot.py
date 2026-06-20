import pandas as pd
import matplotlib.pyplot as plt

def main():
    try:
        # Load the flight data
        df = pd.read_csv('data/flight_data.csv')
        
        # Clean out NaN rows for plotting to avoid matplotlib warnings
        df_clean = df.dropna()

        # Create a figure with 3 subplots
        fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12), sharex=True)

        # Plot Altitude
        ax1.plot(df_clean['time'], df_clean['altitude'], color='b', linewidth=2)
        ax1.set_ylabel('Altitude (m)', color='b')
        ax1.grid(True)
        ax1.set_title('RTOS 6-DOF Aircraft Simulation - Flight Telemetry')

        # Plot Velocity
        ax2.plot(df_clean['time'], df_clean['velocity'], color='g', linewidth=2)
        ax2.set_ylabel('Velocity Y (m/s)', color='g')
        ax2.grid(True)

        # Plot Thrust
        ax3.plot(df_clean['time'], df_clean['acceleration'], color='r', linewidth=2)
        ax3.set_ylabel('Thrust (N)', color='r')
        ax3.set_xlabel('Time (s)')
        ax3.grid(True)

        # Highlight CRUISE phase
        if len(df_clean[df_clean['acceleration'] == 30000]) > 0:
            cruise_start = df_clean[df_clean['acceleration'] == 30000]['time'].iloc[0]
            ax3.axvline(x=cruise_start, color='k', linestyle='--', alpha=0.5, label='CRUISE State Triggered')
            ax3.legend()

        plt.tight_layout()
        plt.savefig('data/flight_plot.png')
        print("Plot saved to data/flight_plot.png")

    except Exception as e:
        print(f"Error reading or plotting data: {e}")

if __name__ == '__main__':
    main()
