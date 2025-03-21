import sys
import traceback

from ci.defs.defs import SYNC
from ci.praktika.gh import GH
from ci.praktika.info import Info
from ci.praktika.utils import Shell


def check():
    info = Info()
    if info.user_name not in ("maxknv", "yariks5s", "kssenii"):
        print(f"Not enabled for [{info.user_name}]")
        return
    if not Shell.check(
        f"gh workflow run private_quick_sync.yml --repo ClickHouse/clickhouse-private --ref ci_fix_quick_sync_for_forks --field pr_number={info.pr_number} --field branch_name={info.git_branch} --field title='{info.pr_title}' --field sha={info.sha}",
    ):
        GH.post_commit_status(
            name=SYNC, status="error", description="failed to start the sync", url=""
        )
    else:
        GH.post_commit_status(
            name=SYNC, status="pending", description="sync started", url=""
        )


if __name__ == "__main__":
    try:
        check()
    except Exception as e:
        print("Failed to initiate sync")
        traceback.print_exc()
        sys.exit(1)
