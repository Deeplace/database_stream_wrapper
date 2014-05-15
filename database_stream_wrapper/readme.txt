Модуль позволяет сохранять файлы в бд, используя функции для работы с файлами

$document_uri = variable_get('documents_save_place', 'db://docs/?id=') . $document->id;
file_put_contents($document_uri, 'data to save')
где будет создана таблица __PREFIX__docs

Перед тем как установить модуль нужно установить переменную variable_set('db_stream_available_tables', '') 
со списком таблиц, которые будут в последствии установлены
  
      
 
