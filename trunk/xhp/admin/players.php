<?php
	require_once('common.php');

	function select_players($where)
	{
		return db_query(sprintf("select p.sid, p.pid, p.pass, p.name as pname, p.flags, c.name as cname, motd, p.mtime, p.ctime from xwi_serials s inner join xwi_players p using (sid) left join xwi_clans c using (cid)%s order by p.name", $where));
	}

	function echo_players($results)
	{
		while ($result = mysql_fetch_array($results))
		{
			$motd = trim($result['motd']);
			printf('<tr><td><a href="?pid=%d">%s</a><td>%s<td>%s<td><a href="logins.php?pid=%d">L</a><td><a href="http://xwis.net/xcl/?pname=%s">P</a><td><a href="http://xwis.net/xcl/?pname=%s">C</a><td align=right><a href="?sid=%d">%d</a><td>', $result[pid], $result[pname], $result[pass] && !($result[flags] & 2) ? "" : "*", $result[cname], $result[pid], $result[pname], $result[cname], $result[sid], $result[sid]);
			printf('<a href="?a=bl_insert&pid=%d">-&gt;BL</a>', $result[pid]);
			printf('<td><a href="?a=rb_insert&pid=%d">-&gt;RB</a>', $result[pid]);
			printf('<td>%s<td>%s', gmdate("d-m-Y", $result[mtime]), gmdate("d-m-Y", $result[ctime]));
			printf('<td><a href="?a=motd&pid=%d">%s</a>', $result['pid'], $motd ? nl2br(htmlspecialchars(substr($motd, 0, 80))) : 'motd');
		}
	}

	function echo_warning($result)
	{
		printf('<tr><td align=right><a href="?a=show_warning&amp;wid=%d">%d</a><td><a href="?pname=%s">%s</a><td>', $result['wid'], $result['wid'], $result['name'], $result['name']);
		if ($result[link])
			printf('<a href="%s">link</a>', htmlspecialchars($result[link]));
		printf("<td>%s<td>%s<td>%s", htmlspecialchars($result[reason]), htmlspecialchars($result[admin]), gmdate("H:i d-m-Y", $result[mtime]));
	}

	function echo_warnings($results)
	{
		while ($result = mysql_fetch_array($results))
			echo_warning($result);
	}

	require('templates/links.php');
	echo('<hr>');
	require('templates/search.php');
	echo('<hr>');
	$pname = trim($_REQUEST[pname]);
	$remote_user = $_SERVER['REMOTE_USER'];
	switch ($_REQUEST['a'])
	{
	case 'chat':
		$results = db_query(sprintf("select * from xwi_chat where `from` like '%s' or `to` like '%s' order by time desc limit 10000", addslashes($pname), addslashes($pname)));
		echo('<table>');
		while ($result = mysql_fetch_array($results))
		{
			printf('<tr><td nowrap>%s<td><a href="?a=chat&amp;pname=%s">%s<td>%s<td><a href="?a=chat&amp;pname=%s">%s',
				gmdate("H:i:s d-m-Y", $result['time']), urlencode($result['from']), htmlspecialchars($result['from']), htmlspecialchars($result['msg']), urlencode($result['to']), htmlspecialchars($result['to']));
		}
		echo('</table>');
		break;
	case 'motd':
	case 'motd_submit':
		$pid = $_REQUEST[pid];
		$results = db_query(sprintf("select p.*, s.motd from xwi_players p inner join xwi_serials s using (sid) where pid = %d", $pid));
		$result = mysql_fetch_array($results);
		$name = $result[name];
		$sid = $result['sid'];

		if ($_REQUEST['a'] == "motd_submit" && name)
		{
			$motd = trim($_REQUEST['motd']);
			db_query(sprintf("update xwi_logins l inner join xwi_serials s using (sid) set s.motd = '%s' where l.pid = %d", addslashes($motd), $pid));
			db_query(sprintf("update xwi_serials set motd = '%s' where sid = %d", addslashes($motd), $sid));
			echo("<b>Updated!</b><hr>");
		}
		else
			$motd = $result[motd];
		require('templates/motd_insert.php');
		break;
	case 'bl_insert':
	case 'bl_insert_submit':
		$pid = $_REQUEST[pid];
		if ($result = mysql_fetch_array(db_query(sprintf("select * from xwi_players where pid = %d", $pid))))
		{
			$sid = $result[sid];
			$name = $result[name];
			$link = trim($_REQUEST['link']);
			$reason = trim($_REQUEST['reason']);
			$dura = $_REQUEST[dura] ? $_REQUEST[dura] : 16;
			if ($_REQUEST['a'] == "bl_insert_submit" && $name && $reason)
			{
				db_query(sprintf("insert into xbl (admin, sid, name, link, reason, duration, mtime, ctime) values ('%s', %d, '%s', '%s', '%s', %d, unix_timestamp(), unix_timestamp())",
					addslashes($remote_user), $sid, $name, addslashes($link), addslashes($reason), $dura));
				// db_query(sprintf("update xwi_serials set wtime = unix_timestamp() + %d where sid in (%s)", 24 * 60 * 60 * $dura, addslashes(implode(",", $sids))));
			}
		}
		require('templates/bl_insert.php');
		echo('<hr>');
		echo('<table>');
		echo_players(select_players(sprintf(" where p.sid = %d", $sid)));
		echo('</table>');
		break;
	case 'rb_insert':
		$pid = $_REQUEST[pid];
		db_query(sprintf("update xwi_players set flags = flags ^ 2 where pid = %d", $pid));
		echo('<table>');
		echo_players(select_players(sprintf(" where pid = %d", $pid)));
		echo('</table>');
		break;
	case 'bad_passes':
		$results = db_query("select flags, name from xwi_players inner join bad_passes using (pass) where ~flags & 2 order by name");
		echo('<table>');
		while ($result = mysql_fetch_array($results))
			printf('<tr><td><a href="?pname=%s">%s</a><td>%s', $result['name'], $result['name'], $result[flags] & 2 ? '*' : '');
		echo('</table>');
		break;
	case 'games':
		$results = db_query(sprintf("select gid, ipa, sid, p.name, c.name cname from xcl_games_players gp inner join xcl_players p using (pid) left join xcl_players c on (gp.cid = c.pid) where p.name like '%s' order by gid desc", $pname));
		echo('<table>');
		while ($result = mysql_fetch_array($results))
		{
			printf('<tr><td align=right><a href="http://xwis.net/xcl/?gid=%d">%d</a><td><a href="logins.php?ipa=%d">%s</a><td align=right><a href="logins.php?sid=%d">%d</a><td>%s<td>%s',
				$result['gid'], $result['gid'], $result['ipa'], long2ip($result['ipa']), $result['sid'], $result['sid'], $result['name'], $result['cname']);
		}
		echo('</table>');
		break;
	case 'invalid_serials':
		$results = db_query("select valid, count(*) c from xwi_serials group by valid");
		echo('<table>');
		while ($result = mysql_fetch_array($results))
			printf("<tr><td align=right>%d<td align=right>%d", $result[c], $result[valid]);
		echo('</table>');
		echo('<hr>');
		$results = db_query("select flags, name from xwi_players inner join xwi_serials using (sid) where ~flags & 2 and valid < 0 order by name");
		echo('<table>');
		while ($result = mysql_fetch_array($results))
			printf('<tr><td><a href="?pname=%s">%s</a><td>%s', $result['name'], $result['name'], $result[flags] & 2 ? '*' : '');
		echo('</table>');
		break;
	case 'xbl':
		$results = db_query("select * from xbl order by wid desc");
		echo('<table>');
		echo_warnings($results);
		echo('</table>');
		echo('<hr>');
		$results = db_query("select * from xbl_ipas order by wid desc");
		echo('<table>');
		while ($result = mysql_fetch_array($results))
		{
			printf('<tr>');
			printf('<td align=right><a href="?a=show_warning&amp;wid=%d">%d</a>', $result['wid'], $result['wid']);
			printf('<td>%s', long2ip($result['ipa']));
			printf('<td>%s', htmlspecialchars($result['creator']));
			printf('<td>%s', gmdate("H:i d-m-Y", $result['ctime']));
		}
		echo('</table>');
		break;
	case 'xwsvs':
		$results = db_query("select * from xwsvs_log order by time desc");
		echo('<table>');
		while ($result = mysql_fetch_array($results))
			printf('<tr><td align=right>%x<td align=right><a href="?sid=%d">%d</a><td>%s<td>%s', $result['gsku'], $result['sid'], $result['sid'], nl2br(htmlspecialchars($result['msg'])), gmdate("H:i d-m-Y", $result['time']));
		echo('</table>');
		break;
	case 'edit_warning_submit':
	case 'edit_warning_delete_ipa_submit':
	case 'edit_warning_insert_ipa_submit':
	case 'edit_warning_delete_serial_submit':
	case 'edit_warning_insert_serial_submit':
	case 'show_warning':
		$wid = $_REQUEST['wid'];
		if ($_REQUEST['a'] == 'edit_warning_submit')
		{
			$duration = $_REQUEST['duration'];
			$link = trim($_REQUEST['link']);
			$motd = trim($_REQUEST['motd']);
			$reason = trim($_REQUEST['reason']);
			db_query(sprintf("update xbl set duration = %d, motd = '%s', link = '%s', reason = '%s', mtime = unix_timestamp() where wid = %d",
				$duration, addslashes($motd), addslashes($link), addslashes($reason), $wid));
		}
		$results = db_query(sprintf("select * from xbl where wid = %d", $wid));
		if ($row = mysql_fetch_array($results))
		{
			$ipa = $_REQUEST['ipa'];
			$sid = 0 + $_REQUEST['sid'];
			switch ($_REQUEST['a'])
			{
			case 'edit_warning_delete_ipa_submit':
				db_query(sprintf("delete from xbl_ipas where ipa = %d and wid = %d", $ipa, $wid));
				break;
			case 'edit_warning_insert_ipa_submit':
				if ($ipa)
					db_query(sprintf("insert ignore into xbl_ipas (wid, ipa, creator, ctime) values (%d, %d, '%s', unix_timestamp())", $wid, ip2long($ipa), addslashes($remote_user)));
				break;
			case 'edit_warning_delete_serial_submit':
				db_query(sprintf("delete from xbl_serials where sid = %d and wid = %d", $sid, $wid));
				break;
			case 'edit_warning_insert_serial_submit':
				if ($sid)
					db_query(sprintf("insert ignore into xbl_serials (wid, sid, creator, ctime) values (%d, %d, '%s', unix_timestamp())", $wid, $sid, addslashes($remote_user)));
				break;
			}
			$creator = $row['admin'];
			$ctime = gmdate("H:i d-m-Y", $row['ctime']);
			$duration = htmlspecialchars($row['duration']);
			$link = htmlspecialchars($row['link']);
			$mtime = gmdate("H:i d-m-Y", $row['mtime']);
			$motd = htmlspecialchars($row['motd']);
			$name = htmlspecialchars($row['name']);
			$reason = htmlspecialchars($row['reason']);
			$sid = $row['sid'];
			include('templates/show_warning.php');
			echo('<hr>');
			echo('<table>');
			$results = db_query(sprintf("select * from xbl_ipas where wid = %d", $wid));
			while ($row = mysql_fetch_array($results))
			{
				echo('<tr>');
				printf('<td><a href="logins.php?ipa=%d">%s</a>', $row['ipa'], long2ip($row['ipa']));
				printf('<td>%s', $row['creator']);
				printf('<td>%s', gmdate("H:i d-m-Y", $row['ctime']));
				printf('<td><a href="?a=edit_warning_delete_ipa_submit&amp;ipa=%d&amp;wid=%d">delete</a>', $row['ipa'], $wid);
			}
			echo('</table>');
			echo('<hr>');
			include('templates/edit_warning_insert_ipa.php');
			echo('<hr>');
			echo('<table>');
			$results = db_query(sprintf("select * from xbl_serials where wid = %d", $wid));
			while ($row = mysql_fetch_array($results))
			{
				echo('<tr>');
				printf('<td align=right><a href="?sid=%d">%d</a>', $row['sid'], $row['sid']);
				printf('<td>%s', $row['creator']);
				printf('<td>%s', gmdate("H:i d-m-Y", $row['ctime']));
				printf('<td><a href="?a=edit_warning_delete_serial_submit&amp;sid=%d&amp;wid=%d">delete</a>', $row['sid'], $wid);
			}
			echo('</table>');
			echo('<hr>');
			include('templates/edit_warning_insert_serial.php');
		}
		break;
	case 'washers':
		$results = db_query("select pid, name, win_count, loss_count, points, count(*) c from xcl_games inner join xcl_games_players using (gid) inner join xcl_players using (pid) where oosy and points group by pid having c >= 4 order by c desc, points desc");
		echo('<table>');
		printf('<tr>');
		printf('<th>Name');
		printf('<th>Washes');
		printf('<th>Wins');
		printf('<th>Losses');
		printf('<th>Points');
		while ($result = mysql_fetch_array($results))
		{
			printf('<tr>');
			printf('<td><a href="?pname=%s">%s</a>', htmlspecialchars($result['name']), htmlspecialchars($result['name']));
			printf('<td align=right>%d', $result['c']);
			printf('<td align=right>%d', $result['win_count']);
			printf('<td align=right>%d', $result['loss_count']);
			printf('<td align=right>%d', $result['points']);
		}
		echo('</table>');
		break;
	default:
		$cname = trim($_REQUEST[cname]);
		$pid = $_REQUEST[pid];
		$sid = $_REQUEST[sid];
		if (is_numeric($pname))
		{
			$sid = $pname;
			unset($pname);
		}
		if ($_REQUEST['a'] == "motds")
			$where = " where motd != ''";
		else if ($cname)
			$where = sprintf(" where c.name like '%s'", addslashes($cname));
		else if ($pid)
			$where = sprintf(" where pid = %d", $pid);
		else if ($pname)
			$where = sprintf(" where p.name like '%s'", addslashes($pname));
		else if ($sid)
			$where = sprintf(" where p.sid = %d", $sid);
		else
			$where = " where ~flags & 2 and wtime > unix_timestamp()";
		echo("<table><tr><th align=left>Player<th><th align=left>Clan<th><th><th><th align=right>SID<th><th><th align=left>Mtime<th align=left>Ctime");
		echo_players(select_players($where));
		echo('</table>');
		if ($sid)
		{
			$results = db_query(sprintf("select * from xwi_serials where sid = %d", $sid));
			if ($result = mysql_fetch_array($results))
			{
				echo("<br><table><tr><th align=right>SID<th align=right>GSKU<th align=right>Valid<th align=left>Wtime<th align=left>Mtime<th align=left>Ctime");
				do
				{
					printf("<tr><td align=right>%d<td align=right>%x<td align=right>%d<td>%s<td>%s<td>%s", $result[sid], $result[gsku], $result[valid], $result[wtime] ? gmdate("H:i d-m-Y", $result[wtime]) : "", gmdate("H:i d-m-Y", $result[mtime]), gmdate("H:i d-m-Y", $result[ctime]));
				}
				while ($result = mysql_fetch_array($results));
				echo('</table>');
			}
			$results = db_query(sprintf("select * from xbl where sid = %d", $sid));
			if ($result = mysql_fetch_array($results))
			{
				echo("<br><table>");
				do
				{
					echo_warning($result);
				}
				while ($result = mysql_fetch_array($results));
				echo('</table>');
			}
		}
	}
	echo('<hr>');
	require('templates/links.php');
?>