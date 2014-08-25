URI received

Is URI item in cache?
	NO {
		Forward to the server and get response
		Is the content larger than MAX_OBJECT_SIZE?
			larger {
				just forward to client
				End
			}
			smaller {
				Will adding the content oversize the cache?
					YES {
						evict LRU item
						insert cache
						forward to client
						END
					}
					NO {
						insert cache
						forward to client
						END
					}
			}
	}
	YES {
		retrieve it from cache;
		forward to client
		END;
	}
