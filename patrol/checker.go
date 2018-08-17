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
    "os/exec"
    "strconv"
    "strings"
    "time"
)

const (
	// DB_CPU_ALL_CORE_HIGH
	rangeLen = 60
	cpuThreshold = 50.0

	// POSTGRES_STAT_ACTIVITY_ACTIVE
	activeLen = 120
	activeThreshold = 80

	// POSTGRES_STAT_ACIVITY_IDLE_IN_TRANSCATION
	idleInTranLen = 60
	idleInTranThreshold = 10

	// PGBOUNCER_POSTGRES_SERVICES_DOWN
	downLen = 3

	// TODO
	// POSTGRES_STAT_ACTIVITY_IDLE_IN_TRANSCATION_ABORTED
	// PGBOUNCER_AVG_QUERY_SLOW
	// RAM_USAGE_HIGH
	// FreeSpaceLessThan10Percentage
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
	log.Info("CpuChecker on work...")

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
					log.Warningf("CUR_CPUUSAGE: %f; AVG_CPUUSAGE: %f\n", curcpuUsage, avgLoad)
					ops_ch <- OpsMessage{OpsType: "killquery", metric: avgLoad}
				} else {
					log.Debugf("CUR_CPUUSAGE: %f; AVG_CPUUSAGE: %f\n", curcpuUsage, avgLoad)
				}
			}
		}
	}
}

func ActivityChecker(ops_ch chan<- OpsMessage) {
	log.Info("ActivityChecker on work...")

	db, err := sql.Open("postgres", c.ConnStr)
	if err != nil {
		  panic(err)
	}
	defer db.Close()
	err = db.Ping()
	if err != nil {
		  panic(err)
	}

	var (
		activeCounts int = 0
		idleInTranCounts int = 0
	)
	for  {
		time.Sleep(1 * time.Second)

		rows, err := db.Query("select state,count(*) from pg_stat_activity group by state having state != '';")
		if err != nil {
			log.Fatal(err)
		}
		defer rows.Close()

		var (
			curActiveCount int = 0
			curidleInTranCount int = 0
		)
		for rows.Next() {
			var (
				state string
				count int
			)
			if err := rows.Scan(&state, &count); err != nil {
                log.Fatal(err)
			}
			switch state {
			case "active":
				curActiveCount = count
				if curActiveCount >= activeThreshold {
					activeCounts ++
					if activeCounts > activeLen {
						log.Warningf("ACTIVITY: %s -> %d\n", state, count)
						ops_ch <- OpsMessage{OpsType: "killquery", metric: float64(count)}
					}
				} else {
					activeCounts = 0
				}
				break;
			case "idle in transaction":
				curidleInTranCount = count
				if curidleInTranCount >= idleInTranThreshold {
					idleInTranCounts ++
					if idleInTranCounts > idleInTranLen {
						log.Warningf("ACTIVITY: %s -> %d\n", state, count)
						ops_ch <- OpsMessage{OpsType: "killquery", metric: float64(count)}
					}
				} else {
					idleInTranCounts = 0
				}
				break;
			default:
				break;
			}
		}
		log.Infof("ACTIVITY: active->%d; idleInTransaction->%d:%d\n", curActiveCount,
		curidleInTranCount, idleInTranCounts)
	}
}

func CheckPGalive(ops_ch chan<- OpsMessage) {
	for  {
		time.Sleep(5 * time.Second)

		command := exec.Command("pg_ctl", "status", "-D",c.PgData)
		err := command.Run()
		if nil != err {
			ops_ch <- OpsMessage{OpsType: "pg_restart", metric: float64(-1)}
			log.Warning("PostgreSQL Service Down!!!");
		}
	}
}

func CheckPGBalive(ops_ch chan<- OpsMessage) {
	for  {
		time.Sleep(5 * time.Second)

		command := exec.Command("pgrep", "pgbouncer")
		err := command.Run()
		if nil != err {
			ops_ch <- OpsMessage{OpsType: "pgb_restart", metric: float64(-1)}
			log.Warning("Pgbouncer Service Down!!!");
		}
	}
}
