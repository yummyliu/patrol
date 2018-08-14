/* =============================================================================
#     FileName: checker.go
#         Desc:
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-08-14 16:09:28
#      History:
============================================================================= */
package main

import (
    "container/list"
    "database/sql"
    _ "github.com/lib/pq"
    "io/ioutil"
    "strconv"
    "strings"
    "time"
)

func getCPUSample() (idle, total uint64) {
    contents, err := ioutil.ReadFile("/proc/stat")
    if err != nil {
        return
    }
    lines := strings.Split(string(contents), "\n")
    for _, line := range(lines) {
        fields := strings.Fields(line)
        if fields[0] == "cpu" {
            numFields := len(fields)
            for i := 1; i < numFields; i++ {
                val, err := strconv.ParseUint(fields[i], 10, 64)
                if err != nil {
					log.Errorf("%s %s\n" , i, fields[i], err)
                }
                total += val
                if i == 4 {
					// idle is the 5th field in the cpu line
                    idle = val
                }
            }
            return
        }
    }
    return
}

func cpuloadrate(idleTicks, totalTicks float64) float64 {
	return 100 * (totalTicks - idleTicks) / totalTicks
}

func CpuChecker(ops_ch chan<- OpsMessage) {
	log.Info("cpu checker on work...")

	// stats collect
	sumCpuLoads := list.New()
	preidle, pretotal := getCPUSample()
	sumLoad := 0.0
	for  {
		time.Sleep(1 * time.Second)
    	curidle, curtotal := getCPUSample()

    	curcpuUsage := cpuloadrate(float64(curidle - preidle), float64(curtotal - pretotal))

		// put cur load into list and add it into sum
		sumLoad += curcpuUsage;
		sumCpuLoads.PushBack(curcpuUsage)

		if sumCpuLoads.Len() > rangeLen {
			if cur, ok := sumCpuLoads.Front().Value.(float64); ok {

				// remove oldest load outof list and cut it outof sum
				sumCpuLoads.Remove(sumCpuLoads.Front())
				sumLoad -= cur

				avgLoad := (sumLoad/float64(rangeLen))
				if avgLoad > cpuThreshold {
					log.Warningf("danger!,CPU usage is %f%% avg usage is %f%%\n", curcpuUsage, avgLoad)
					ops_ch <- OpsMessage{OpsType: "killquery", metric: avgLoad}
				} else {
					log.Debugf("safe~~~,CPU usage is %f%% avg usage is %f%%\n", curcpuUsage, avgLoad)
				}
			}
		}
	}
}

func activityCheck(ops_ch chan<- OpsMessage) {
	log.Info("go activity check")

	db, err := sql.Open("postgres", connStr)
	if err != nil {
		  panic(err)
	}
	defer db.Close()
	err = db.Ping()
	if err != nil {
		  panic(err)
	}

	counts := 0
	for  {
		time.Sleep(1 * time.Second)

		var count int
		err := db.QueryRow("select count(*) as count from pg_stat_activity where state = 'active';").Scan(&count)
		switch {
		case err == sql.ErrNoRows:
		        log.Panic("0 row")
		case err != nil:
		        log.Panic(err)
		default:
			if count > activeThreshold{
				counts +=1
				if counts > activeLen {
					log.Warningf("danger!,active count is %d%%\n", counts )
					ops_ch <- OpsMessage{OpsType: "killquery", metric: float64(count)}
				}
			} else {
				counts = 0
				log.Debugf("safe~~~,count %d querys\n",count)
			}
		}
	}
}
