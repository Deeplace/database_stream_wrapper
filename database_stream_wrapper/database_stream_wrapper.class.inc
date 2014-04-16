<?php

class DBStream {
  const MAX_PACK_SIZE = 10096;
  const DB_PREFIX = 'db_signer_';

  private $_ps;
  private $_rowId = 0;
  private $position = 1;
  private $buffer = '';

  function stream_open($path, $mode, $options = 0, &$opath = '') {
    $url = parse_url($path);
    $this->db = static::DB_PREFIX . $url['host'];
    parse_str($url['query'], $query);

    $this->_rowId = isset($query['id']) ? $query['id'] : '';
    if(empty($this->_rowId)) return FALSE;

    if($mode == 'w' || $mode == 'wb') {
      if(!db_table_exists($this->db)) {
        return FALSE;
      }
      db_delete($this->db)
        ->condition('id', $this->_rowId)
        ->execute();
    }

    return TRUE;
  }

  public function stream_lock($operation) {
    return TRUE;
  }

  function stream_read($count) {
    if(!db_table_exists($this->db)) {
      return 0;
    }
    $this->buffer = db_query("SELECT substring(dt.data, :f, :t)  as d FROM {" . $this->db . "} dt WHERE id = :id",
                   array(':f' => $this->position, ':t' => $count, ':id' => $this->_rowId))->fetchField();
    $this->position+=$count;
    return !$this->buffer ? '' : $this->buffer;
  }

  function stream_write($data) {
    if(!db_table_exists($this->db)) {
      return 0;
    }
    if($this->position !== 1) {
      db_query(
        'UPDATE {'.$this->db.'} SET data = CONCAT(data, :s) WHERE id = :id',
        array(':id' => $this->_rowId, ':s' => $data)
      );
    }
    else {
      db_insert($this->db)->fields(
        array(
          'id' => $this->_rowId,
          'data' => $data,
          'created' => time(),
        )
      )->execute();
    }

    $this->position += strlen($data);
    return strlen($data);
  }

  function stream_tell()
  {
    return $this->position;
  }

  function stream_eof() {
    return TRUE; // if stream_write returns non-empty result, this will be ignored
  }

  function stream_seek($offset, $whence) {
        //No need to be implemented
  }

  function stream_close() {
    unset($this->_ps);
  }

  function url_stat($path, $flags) {
    $this->stream_open($path, 'r');

    if(!db_table_exists($this->db)) {
      return array();
    }
    $data = db_select($this->db, 'dt')->fields('dt')->condition('id', $this->_rowId)->execute()
    ->fetch();

    return array(
      'dev' => -1,
      'ino' => 0,
      'mode' => 0100644,
      'nlink' => 0,
      'uid' => 0,
      'gid' => 0,
      'rdev' => 0,
      'size' => strlen($data->data),
      'atime' => $data->created,
      'mtime' => $data->created,
      'ctime' => $data->created,
      'blksize' => 0,
      'blocks' => 0,
      );
  }

  function stream_stat() {
    return array(
      'dev' => -1,
      'ino' => 0,
      'mode' => 0100644,
      'nlink' => 0,
      'uid' => 0,
      'gid' => 0,
      'rdev' => 0,
      'size' => 0,
      'atime' => time(),
      'mtime' => time(),
      'ctime' => time(),
      'blksize' => 0,
      'blocks' => 0,
      );
  }

  function unlink($uri) {
    $url = parse_url($uri);

    parse_str($url['query'], $query);

    if(!db_table_exists(static::DB_PREFIX . $url['host'])) {
      return FALSE;
    }

    return (bool) db_delete(static::DB_PREFIX . $url['host'])
                  ->condition('id', $query['id'])
                  ->execute();
  }
}

